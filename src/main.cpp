#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

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
#include "orderbook/orderbook_event.h"
#include "pubsub/pub_worker.h"
#include "quill/LogMacros.h"
#include "storage/storage_router.h"
#include "telemetry/prometheus_exposer.h"
#include "telemetry/telemetry_agent.h"

int main(int argc, char* argv[]) {
  using namespace sqc;
  std::string config_path = "config/config.yaml";
  if(argc > 1) config_path = argv[1];

  Config::Load(config_path);
  InitLogger();
  LOG_INFO(GetLogger(), "smart_quant_collector v1.0.0 starting");

  SignalHandler::Install();

  // 0. Shared ZMQ context
  zmq::context_t zmq_ctx(1);

  // 1. Channel registry
  ChannelRegistry channel_registry;

  // 2. Storage
  StorageRouter storage_router;

  // 3. Telemetry
  PrometheusExposer prometheus;
  TelemetryAgent telemetry_agent(&prometheus);

  auto num_parsers = Config::Instance().threading_matrix.parser_cores.size();
  if(num_parsers == 0) {
    LOG_CRITICAL(GetLogger(), "parser_cores must not be empty — modulo-by-zero on shard dispatch");
    return EXIT_FAILURE;
  }

  // Per-shard telemetry slots — each parser thread writes its own slot,
  // eliminating writer-writer contention on the seqlock.
  std::vector<TelemetrySlot> telemetry_slots(num_parsers);

  // 4. Boost.Asio + SSL
  net::io_context io_ctx;
  net::ssl::context ssl_ctx(net::ssl::context::tlsv12_client);
  ssl_ctx.set_verify_mode(net::ssl::verify_peer);
  ssl_ctx.set_default_verify_paths();

  // 5. Symbol channels
  std::vector<std::shared_ptr<ShardQueue>> shard_queues;
  for(size_t i = 0; i < num_parsers; ++i) shard_queues.push_back(std::make_shared<ShardQueue>(4096));

  std::vector<std::shared_ptr<SymbolChannel>> channels;
  std::vector<std::string> channel_topics;  // channel_id → topic prefix

  for(const auto& ex : Config::Instance().exchanges) {
    if(!ex.enabled) continue;
    for(const auto& ch : ex.channels) {
      const auto* adapter = GetAdapter(ex.name, ParseChannelType(ch.type));
      if(!adapter) {
        LOG_ERROR(GetLogger(), "Unknown exchange: {}, skipping", ex.name);
        continue;
      }

      for(const auto& sym : ch.symbols) {
        if(!sym.enabled) continue;

        ChannelInfo info;
        info.exchange = ex.name;
        info.type = adapter->channel_type;
        info.symbol = sym.name;
        info.depth_level = sym.depth_level;
        uint32_t id = channel_registry.Register(info);

        // Build topic prefix: "exchange:type:symbol"
        if(id >= channel_topics.size()) {
          channel_topics.resize(id + 1);
        }
        channel_topics[id] = std::string(ex.name) + ":" + ChannelTypeName(adapter->channel_type) + ":" + sym.name;

        // Register channel for storage — creates CSV writer / mmap engine
        // keyed by uint32_t channel_id for zero-allocation hot-path lookup.
        storage_router.RegisterChannel(id, info, sym.persist_to_disk);

        auto chan = std::make_shared<SymbolChannel>(adapter, sym.name, sym.depth_level, id, io_ctx, ssl_ctx, shard_queues);
        channels.push_back(chan);
      }
    }
  }

  // Pub/Sub
  const auto& pub_cfg = Config::Instance().pub;
  PubWorker pub_worker(zmq_ctx, channel_topics, num_parsers, pub_cfg.tcp_endpoint, pub_cfg.ipc_endpoint);

  // Log subscription summary
  LOG_INFO(GetLogger(), "=== Subscription Summary ===");
  LOG_INFO(GetLogger(), "Channels: {}, Parser shards: {}", channels.size(), num_parsers);
  for(const auto& ex : Config::Instance().exchanges) {
    if(!ex.enabled) continue;
    for(const auto& ch : ex.channels) {
      for(const auto& sym : ch.symbols) {
        if(!sym.enabled) continue;
        LOG_INFO(GetLogger(), "  {}:{}:{}:tick", ex.name, ch.type, sym.name);
        LOG_INFO(GetLogger(), "  {}:{}:{}:depth", ex.name, ch.type, sym.name);
        LOG_INFO(GetLogger(), "  {}:{}:{}:book_ticker", ex.name, ch.type, sym.name);
      }
    }
  }
  LOG_INFO(GetLogger(), "ZMQ PUB: tcp={} ipc={}", pub_cfg.tcp_endpoint, pub_cfg.ipc_endpoint);

  // Freeze channel registrations before parser threads start.
  storage_router.FreezeChannels();

  // 6. Parser workers
  std::vector<std::unique_ptr<ShardParserWorker>> parser_workers;
  for(size_t i = 0; i < num_parsers; ++i) {
    auto worker = std::make_unique<ShardParserWorker>(
        Config::Instance().threading_matrix.parser_cores[i], *shard_queues[i],
        [&, i](TickData tick) -> void {
          const auto* info = channel_registry.Lookup(tick.channel_id);
          if(!info) return;
          uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          tick.local_diff = now_ns - tick.local_diff;
          std::memset(tick.padding, 0, sizeof(tick.padding));
          storage_router.RouteTick(tick, *info);
          pub_worker.PublishTick(tick, i);
          // pub_worker.dropped_count() reads atomics concurrently — approximate,
          // single-value snapshot is acceptable for a telemetry gauge.
          WriteTelemetrySlot(&telemetry_slots[i], tick.local_diff, shard_queues[i]->size(), 0, pub_worker.dropped_count());
        },
        [&, i](uint32_t channel_id, const DepthUpdateEvent& event) -> void {
          const auto* info = channel_registry.Lookup(channel_id);
          if(!info) return;
          uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          uint64_t latency_ns = now_ns - event.local_diff;
          storage_router.RouteOrderbook(event, latency_ns, *info);
          pub_worker.PublishDepth(event, i);
          WriteTelemetrySlot(&telemetry_slots[i], latency_ns, shard_queues[i]->size(), 0, pub_worker.dropped_count());
        },
        [&, i](uint32_t channel_id, BookTickerEvent event) {
          const auto* info = channel_registry.Lookup(channel_id);
          if(!info) return;
          uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          event.local_diff = now_ns - event.local_diff;
          storage_router.RouteBookTicker(event, *info);
          pub_worker.PublishBookTicker(event, i);
          WriteTelemetrySlot(&telemetry_slots[i], event.local_diff, shard_queues[i]->size(), 0, pub_worker.dropped_count());
        });

    parser_workers.push_back(std::move(worker));
  }

  // Register per-shard telemetry slots before starting threads.
  for(auto& slot : telemetry_slots) telemetry_agent.RegisterSlot("parser", &slot);

  // 7. Threads
  std::thread telemetry_thread([&]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.telemetry_core);
    telemetry_agent.Run();
  });

  std::vector<std::thread> parser_threads;
  for(size_t i = 0; i < num_parsers; ++i)
    parser_threads.emplace_back([&, i]() {
      if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.parser_cores[i]);
      parser_workers[i]->Run();
    });

  std::thread network_thread([&]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.network_core);
    for(auto& ch : channels) ch->Start();
    auto work_guard = net::make_work_guard(io_ctx);
    io_ctx.run();
  });

  std::thread pub_thread([&]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.pub_core);
    pub_worker.Run();
  });

  LOG_INFO(GetLogger(), "All threads started, waiting for shutdown");

  // 8. Shutdown
  SignalHandler::WaitForShutdown();
  LOG_INFO(GetLogger(), "Shutdown signal received");

  for(auto& ch : channels) ch->Stop();
  io_ctx.stop();

  for(auto& q : shard_queues) q->PushPoisonPill();
  for(auto& t : parser_threads)
    if(t.joinable()) t.join();
  if(network_thread.joinable()) network_thread.join();

  storage_router.FlushAndClose();

  telemetry_agent.Stop();
  if(telemetry_thread.joinable()) telemetry_thread.join();
  pub_worker.Stop();
  if(pub_thread.joinable()) pub_thread.join();
  zmq_ctx.shutdown();

  LOG_INFO(GetLogger(), "smart_quant_collector shutdown complete");
  return 0;
}
