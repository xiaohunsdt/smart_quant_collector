#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "common/cpu_affinity.h"
#include "common/logger_init.h"
#include "common/signal_handler.h"
#include "config/config_loader.h"
#include "exchange/channel_mapping.h"
#include "exchange/crypto/crypto_factory.h"
#include "exchange/rithmic/rithmic_process_manager.h"
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

  // 1. Singletons — touch them here to force construction during single-threaded init.
  ChannelRegistry::Instance();
  StorageRouter::Instance();

  // 2. Telemetry
  PrometheusExposer prometheus;
  TelemetryAgent telemetry_agent(&prometheus);

  auto num_parsers = Config::Instance().threading_matrix.parser_cores.size();
  if(num_parsers == 0) {
    LOG_CRITICAL(GetLogger(), "parser_cores must not be empty — modulo-by-zero on shard dispatch");
    return EXIT_FAILURE;
  }

  // 3. Boost.Asio + SSL
  net::io_context io_ctx;
  net::ssl::context ssl_ctx(net::ssl::context::tlsv12_client);
  ssl_ctx.set_verify_mode(net::ssl::verify_peer);
  ssl_ctx.set_default_verify_paths();

  // 4. Symbol channels — register all crypto symbols into singletons
  auto crypto = BuildCryptoChannels(num_parsers, io_ctx, ssl_ctx);

  // 5. Pub/Sub — topic prefixes are now embedded in ChannelInfo::topic_prefix
  const auto& pub_cfg = Config::Instance().pub;
  PubWorker pub_worker(zmq_ctx, num_parsers, pub_cfg.tcp_endpoint, pub_cfg.ipc_endpoint);

  // Log subscription summary
  LOG_INFO(GetLogger(), "=== Subscription Summary ===");
  LOG_INFO(GetLogger(), "Channels: {}, Parser shards: {}", crypto.channels.size(), num_parsers);
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
  StorageRouter::Instance().FreezeChannels();

  // 5b. Rithmic — cross-process pipeline (shared memory MPSC queue)
  //      Managed by exchange_factory; returns nullptr if no futures exchanges configured.
  auto rithmic_mgr = CreateRithmicManager(config_path, pub_worker);

  // 6. Parser pool — one worker per shard, each bound to its own telemetry slot.
  auto parser_pool = BuildParserPool(crypto, pub_worker, telemetry_agent);

  // 7. Threads
  std::thread telemetry_thread([&]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.telemetry_core);
    telemetry_agent.Run();
  });

  parser_pool.Run();

  std::thread network_thread([&]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.network_core);
    crypto.Start();
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

  crypto.Stop();
  io_ctx.stop();

  // Rithmic shutdown
  if (rithmic_mgr) rithmic_mgr->Shutdown();

  crypto.DrainQueues();
  parser_pool.Shutdown();
  if(network_thread.joinable()) network_thread.join();

  StorageRouter::Instance().FlushAndClose();

  telemetry_agent.Stop();
  if(telemetry_thread.joinable()) telemetry_thread.join();
  pub_worker.Stop();
  if(pub_thread.joinable()) pub_thread.join();
  zmq_ctx.shutdown();

  LOG_INFO(GetLogger(), "smart_quant_collector shutdown complete");
  return 0;
}
