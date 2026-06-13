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
#include "pubsub/pub_worker.h"
#include "quill/LogMacros.h"
#include "storage/storage_router.h"
#include "telemetry/telemetry_agent.h"

namespace sqc {

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type) {
  if (exchange_name == "binance")
    return channel_type == ChannelType::Spot ? &kBinanceSpotAdapter : &kBinancePerpetualAdapter;
  if (exchange_name == "gateio")
    return channel_type == ChannelType::Spot ? &kGateioSpotAdapter : &kGateioPerpetualAdapter;
  // "rithmic" is handled via CreateRithmicManager() (callback-driven, not WebSocket)
  return nullptr;
}

CryptoChannels BuildCryptoChannels(size_t num_parsers,
                                   net::io_context& io_ctx,
                                   net::ssl::context& ssl_ctx) {
  CryptoChannels result;
  for (size_t i = 0; i < num_parsers; ++i)
    result.shard_queues.push_back(std::make_shared<ShardQueue>(4096));

  for (const auto& ex : Config::Instance().exchanges) {
    if (!ex.enabled) continue;
    for (const auto& ch : ex.channels) {
      const auto* adapter = GetAdapter(ex.name, ParseChannelType(ch.type));
      if (!adapter) {
        LOG_ERROR(GetLogger(), "Unknown exchange: {}, skipping", ex.name);
        continue;
      }
      for (const auto& sym : ch.symbols) {
        if (!sym.enabled) continue;

        ChannelInfo info;
        info.exchange = ex.name;
        info.type = adapter->channel_type;
        info.symbol = sym.name;
        info.depth_level = sym.depth_level;
        uint32_t id = ChannelRegistry::Instance().Register(info);

        // Register channel for storage — creates CSV writer / mmap engine
        // keyed by uint32_t channel_id for zero-allocation hot-path lookup.
        StorageRouter::Instance().RegisterChannel(id, info, sym.persist_to_disk);

        result.channels.push_back(std::make_shared<SymbolChannel>(
            adapter, sym.name, sym.depth_level, id, io_ctx, ssl_ctx, result.shard_queues));
      }
    }
  }
  return result;
}

CryptoParserPool BuildParserPool(const CryptoChannels& crypto,
                                 PubWorker& pub_worker,
                                 TelemetryAgent& telemetry_agent) {
  const size_t n = crypto.shard_queues.size();
  CryptoParserPool pool;
  // std::atomic is not movable; allocate the telemetry slot array directly
  // so no reallocation (and no element-move) ever occurs.
  pool.telemetry_slots = std::make_unique<TelemetrySlot[]>(n);
  pool.num_slots = n;

  for (size_t i = 0; i < n; ++i) {
    DataDispatcher dispatcher{pub_worker, &pool.telemetry_slots[i],
                              crypto.shard_queues[i].get(), i};
    pool.workers.push_back(std::make_unique<ShardParserWorker>(
        Config::Instance().threading_matrix.parser_cores[i],
        *crypto.shard_queues[i],
        [dispatcher](TickData tick) { dispatcher.OnTick(std::move(tick)); },
        [dispatcher](uint32_t cid, const DepthUpdateEvent& ev) { dispatcher.OnDepth(cid, ev); },
        [dispatcher](uint32_t cid, BookTickerEvent ev) { dispatcher.OnBookTicker(cid, std::move(ev)); }));

    telemetry_agent.RegisterSlot("parser", &pool.telemetry_slots[i]);
  }
  return pool;
}

void CryptoParserPool::Run() {
  for (size_t i = 0; i < workers.size(); ++i) {
    // Capture raw pointer: safe because workers live in the pool's unique_ptr
    // vector, which outlives all threads (Shutdown() joins before destruction).
    auto* worker = workers[i].get();
    threads.emplace_back([worker, i]() {
      if (Config::Instance().global.cpu_affinity)
        PinToCore(Config::Instance().threading_matrix.parser_cores[i]);
      worker->Run();
    });
  }
}

void CryptoParserPool::Shutdown() {
  for (auto& t : threads)
    if (t.joinable()) t.join();
  threads.clear();
}

}  // namespace sqc
