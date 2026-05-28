#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>

#include "common/cpu_affinity.h"
#include "common/logger_init.h"
#include "common/signal_handler.h"
#include "common/telemetry_slot.h"
#include "config/config_loader.h"
#include "exchange/channel_mapping.h"
#include "exchange/exchange_adapter.h"
#include "exchange/shard_parser_worker.h"
#include "exchange/shard_queue.h"
#include "exchange/symbol_channel.h"
#include "orderbook/local_lob.h"
#include "orderbook/lockstep_fsm.h"
#include "orderbook/orderbook_manager.h"
#include "pubsub/gateway_supervisor.h"
#include "pubsub/pub_worker.h"
#include "quill/LogMacros.h"
#include "storage/storage_router.h"
#include "telemetry/prometheus_exposer.h"
#include "telemetry/telemetry_agent.h"

int main(int argc, char* argv[]) {
  using namespace sqc;
  std::string config_path = "config/config.yaml";
  if (argc > 1) config_path = argv[1];

  auto config = LoadConfig(config_path);
  InitLogger(config.global.log_file_path, config.global.log_level);
  LOG_INFO(GetLogger(), "smart_quant_collector v1.0.0 starting");

  SignalHandler::Install();

  // 1. Channel registry
  ChannelRegistry channel_registry;
  OrderbookManager orderbook_manager;

  // 2. Shard queues
  auto num_parsers = config.threading_matrix.parser_cores.size();
  std::vector<std::shared_ptr<ShardQueue>> shard_queues;
  for (size_t i = 0; i < num_parsers; ++i)
    shard_queues.push_back(std::make_shared<ShardQueue>());

  // 3. Storage
  StorageRouter storage_router(config.storage, config.exchanges);

  // 4. Telemetry + Pub/Sub
  PrometheusExposer prometheus(config.telemetry.listen_port);
  TelemetryAgent telemetry_agent(&prometheus, config.telemetry.report_interval_ms);
  TelemetrySlot telemetry_slot;
  PubWorker pub_worker(config.gateway.unified_pub_endpoint);
  pub_worker.Init(num_parsers);
  GatewaySupervisor gateway_supervisor(config.gateway.internal_router);

  // 5. Boost.Asio + SSL
  net::io_context io_ctx;
  net::ssl::context ssl_ctx(net::ssl::context::tlsv12_client);
  ssl_ctx.set_verify_mode(net::ssl::verify_none);

  // 6. Symbol channels (one per symbol — register + create in single pass)
  std::vector<std::shared_ptr<SymbolChannel>> channels;
  for (const auto& ex : config.exchanges) {
    if (!ex.enabled) continue;
    const auto* adapter = GetAdapter(ex.name);
    if (!adapter) {
      LOG_ERROR(GetLogger(), "Unknown exchange: {}, skipping", ex.name);
      continue;
    }
    for (const auto& ch : ex.channels) {
      std::string rest_host(adapter->endpoints(ch.type).rest_host);

      for (const auto& sym : ch.symbols) {
        if (!sym.enabled) continue;

        ChannelInfo info;
        info.exchange = ex.name;
        info.type = ch.type;
        info.symbol = sym.name;
        uint32_t id = channel_registry.Register(info);
        orderbook_manager.RegisterChannel(id, sym.depth_level, adapter->snapshot_mode);
        orderbook_manager.SetChannelInfo(id, {rest_host, ch.type, sym.name, sym.depth_level, adapter->fetch_snapshot});

        auto chan = std::make_shared<SymbolChannel>(
            adapter, ch.type, sym.name, sym.depth_level, id,
            io_ctx, ssl_ctx, shard_queues);
        channels.push_back(chan);
      }
    }
  }

  // 7. Parser workers
  std::vector<std::unique_ptr<ShardParserWorker>> parser_workers;
  for (size_t i = 0; i < num_parsers; ++i) {
    auto worker = std::make_unique<ShardParserWorker>(config.threading_matrix.parser_cores[i], *shard_queues[i],
      [&](TickData tick) -> void {
      const auto* info = channel_registry.Lookup(tick.channel_id);
      const std::string_view exchange = info ? std::string_view{info->exchange} : std::string_view{};
      const std::string_view type = info ? std::string_view{info->type} : std::string_view{};
      uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      tick.local_timestamp = now_ns - tick.local_timestamp;  // receive→disk latency
      storage_router.RouteTick(tick, exchange, type);
      pub_worker.PublishTick(tick, i);
      WriteTelemetrySlot(&telemetry_slot, 0, 0, 0, pub_worker.dropped_count());
        }, 
        [&](uint32_t channel_id, const DepthUpdateEvent& event) -> void {
          orderbook_manager.OnDepthEvent(channel_id, event);
          auto* lob = orderbook_manager.GetLOB(channel_id);
          if (lob) {
            const auto* info = channel_registry.Lookup(channel_id);
            const std::string_view exchange = info ? std::string_view{info->exchange} : std::string_view{};
            const std::string_view type = info ? std::string_view{info->type} : std::string_view{};
            uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            uint64_t latency_ns = now_ns - event.local_timestamp;
            storage_router.RouteOrderbook(*lob, event.exchange_timestamp, latency_ns,
                                          event.symbol, lob->depth_level(), exchange, type);
          }
        }, 
        [&](uint32_t channel_id, const BookTickerEvent& event) -> bool {
          bool changed = orderbook_manager.OnBookTicker(channel_id, event);
          if (changed) {
            auto* lob = orderbook_manager.GetLOB(channel_id);
            if (lob) {
              const auto* info = channel_registry.Lookup(channel_id);
              const std::string_view exchange = info ? std::string_view{info->exchange} : std::string_view{};
              const std::string_view type = info ? std::string_view{info->type} : std::string_view{};
              uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
              uint64_t latency_ns = now_ns - event.local_timestamp;
              storage_router.RouteOrderbook(*lob, event.exchange_timestamp, latency_ns,
                                            event.symbol, lob->depth_level(), exchange, type);
            }
          }
          return changed;
        });

    parser_workers.push_back(std::move(worker));
  }

  // 8. Threads
  gateway_supervisor.Start();

  std::thread telemetry_thread([&]() {
    if (config.global.cpu_affinity) PinToCore(config.threading_matrix.telemetry_core);
    telemetry_agent.Run();
  });

  std::thread storage_thread([&]() {
    if (config.global.cpu_affinity) PinToCore(config.threading_matrix.storage_core);
    while (!SignalHandler::IsShutdownRequested())
      std::this_thread::sleep_for(std::chrono::milliseconds(config.storage.dolphindb.flush_interval_ms));
  });

  std::vector<std::thread> parser_threads;
  for (size_t i = 0; i < num_parsers; ++i)
    parser_threads.emplace_back([&, i]() {
      if (config.global.cpu_affinity) PinToCore(config.threading_matrix.parser_cores[i]);
      parser_workers[i]->Run();
    });

  std::thread network_thread([&]() {
    if (config.global.cpu_affinity) PinToCore(config.threading_matrix.network_core);
    for (auto& ch : channels) ch->Start();
    auto work_guard = net::make_work_guard(io_ctx);
    io_ctx.run();
  });

  std::thread pub_thread([&]() {
    if (config.global.cpu_affinity) PinToCore(config.threading_matrix.pub_core);
    pub_worker.Run();
  });

  LOG_INFO(GetLogger(), "All threads started, waiting for shutdown");

  // 9. Shutdown
  SignalHandler::WaitForShutdown();
  LOG_INFO(GetLogger(), "Shutdown signal received");

  for (auto& ch : channels) ch->Stop();
  io_ctx.stop();

  for (auto& q : shard_queues) q->PushPoisonPill();
  for (auto& t : parser_threads)
    if (t.joinable()) t.join();
  if (network_thread.joinable()) network_thread.join();

  storage_router.FlushAndClose();

  telemetry_agent.Stop();
  if (telemetry_thread.joinable()) telemetry_thread.join();
  pub_worker.Stop();
  if (pub_thread.joinable()) pub_thread.join();
  if (storage_thread.joinable()) storage_thread.join();
  gateway_supervisor.Stop();

  LOG_INFO(GetLogger(), "smart_quant_collector shutdown complete");
  return 0;
}
