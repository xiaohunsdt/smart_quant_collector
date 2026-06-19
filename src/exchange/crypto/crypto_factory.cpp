#include "exchange/crypto/crypto_factory.h"

#include "common/cpu_affinity.h"
#include "common/logger_init.h"
#include "config/config_loader.h"
#include "exchange/channel_mapping.h"
#include "exchange/crypto/binance/binance_perpetual.h"
#include "exchange/crypto/binance/binance_spot.h"
#include "exchange/crypto/gateio/gateio_perpetual.h"
#include "exchange/crypto/gateio/gateio_spot.h"
#include "exchange/data_dispatcher.h"
#include "quill/LogMacros.h"
#include "storage/storage_router.h"
#include "telemetry/telemetry_agent.h"

namespace sqc {

namespace {
// Per-parser bounded queue capacity. Sized to absorb network bursts without
// heap allocation on the hot path; bounded so a stalled parser back-pressures
// the network thread rather than growing unbounded.
constexpr size_t kShardQueueCapacity = 4096;
}  // namespace

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type) {
  if(exchange_name == "binance") return channel_type == ChannelType::Spot ? &kBinanceSpotAdapter : &kBinancePerpetualAdapter;
  if(exchange_name == "gateio") return channel_type == ChannelType::Spot ? &kGateioSpotAdapter : &kGateioPerpetualAdapter;
  // "rithmic" is handled via CreateRithmicManager() (callback-driven, not WebSocket)
  return nullptr;
}

CryptoChannels BuildCryptoChannels(size_t num_parsers, net::io_context& io_ctx, net::ssl::context& ssl_ctx) {
  CryptoChannels result;
  for(size_t i = 0; i < num_parsers; ++i) result.shard_queues.push_back(std::make_shared<ShardQueue>(kShardQueueCapacity));

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
        uint32_t id = ChannelRegistry::Instance().Register(info);

        // Register channel for storage — creates CSV writer / mmap engine
        // keyed by uint32_t channel_id for zero-allocation hot-path lookup.
        StorageRouter::Instance().RegisterChannel(id, info, sym.persist_to_disk);

        result.channels.push_back(std::make_shared<SymbolChannel>(adapter, sym.name, sym.depth_level, id, io_ctx, ssl_ctx, result.shard_queues));
      }
    }
  }
  return result;
}

CryptoParserPool BuildParserPool(const CryptoChannels& crypto) {
  const size_t n = crypto.shard_queues.size();
  CryptoParserPool pool;
  // std::atomic is not movable; allocate the telemetry slot array directly
  // so no reallocation (and no element-move) ever occurs.
  pool.telemetry_slots = std::make_unique<TelemetrySlot[]>(n);
  pool.num_slots = n;

  for(size_t i = 0; i < n; ++i) {
    DataDispatcher dispatcher{&pool.telemetry_slots[i], crypto.shard_queues[i].get(), i};
    pool.workers.push_back(std::make_unique<ShardParserWorker>(
        Config::Instance().threading_matrix.parser_cores[i], *crypto.shard_queues[i],
        [dispatcher](TickData tick) { dispatcher.OnTick(std::move(tick)); },
        [dispatcher](uint32_t cid, const DepthUpdateEvent& ev) { dispatcher.OnDepth(cid, ev); },
        [dispatcher](uint32_t cid, BookTickerEvent ev) { dispatcher.OnBookTicker(cid, std::move(ev)); }));

    TelemetryAgent::Instance().RegisterSlot(&pool.telemetry_slots[i]);
  }
  return pool;
}

void CryptoParserPool::Run() {
  for(size_t i = 0; i < workers.size(); ++i) {
    // Capture raw pointer: safe because workers live in the pool's unique_ptr
    // vector, which outlives all threads (Shutdown() joins before destruction).
    auto* worker = workers[i].get();
    threads.emplace_back([worker, i]() {
      if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.parser_cores[i]);
      worker->Run();
    });
  }
}

void CryptoParserPool::Shutdown() {
  for(auto& t : threads)
    if(t.joinable()) t.join();
  threads.clear();
}

// ---------------------------------------------------------------------------
// CryptoSubsystem
// ---------------------------------------------------------------------------

std::optional<CryptoSubsystem> BuildCryptoSubsystem() {
  const size_t num_parsers = Config::Instance().threading_matrix.parser_cores.size();
  if(num_parsers == 0) {
    LOG_CRITICAL(GetLogger(), "parser_cores must not be empty — modulo-by-zero on shard dispatch");
    return std::nullopt;
  }

  CryptoSubsystem sys;
  sys.io_ctx = std::make_unique<net::io_context>();
  sys.ssl_ctx = std::make_unique<net::ssl::context>(net::ssl::context::tlsv12_client);
  sys.ssl_ctx->set_verify_mode(net::ssl::verify_peer);
  sys.ssl_ctx->set_default_verify_paths();
  // BuildCryptoChannels calls ChannelRegistry::Register, which throws on a
  // genuine channel_id collision (two channels hashing to the same 32-bit id).
  // Catch so the misconfiguration surfaces as a clean std::nullopt (handled by
  // main's EXIT_FAILURE path) instead of an uncaught exception → std::terminate.
  try {
    sys.channels = BuildCryptoChannels(num_parsers, *sys.io_ctx, *sys.ssl_ctx);
  } catch(const std::exception& e) {
    LOG_CRITICAL(GetLogger(), "Crypto channel registration failed: {}", e.what());
    return std::nullopt;
  }
  sys.parser_pool = BuildParserPool(sys.channels);
  return sys;
}

void CryptoSubsystem::RunParsers() { parser_pool.Run(); }

void CryptoSubsystem::RunNetwork() {
  network_thread_ = std::thread([this]() {
    if(Config::Instance().global.cpu_affinity) PinToCore(Config::Instance().threading_matrix.network_core);
    channels.Start();
    auto work_guard = net::make_work_guard(*io_ctx);
    io_ctx->run();
  });
}

void CryptoSubsystem::StopNetwork() {
  channels.Stop();
  io_ctx->stop();
}

void CryptoSubsystem::DrainQueues() { channels.DrainQueues(); }

void CryptoSubsystem::Shutdown() {
  if(network_thread_.joinable()) network_thread_.join();
  parser_pool.Shutdown();
}

}  // namespace sqc
