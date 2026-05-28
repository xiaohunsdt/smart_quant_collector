#include "storage_router.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/orderbook/local_lob.h"

namespace sqc {

std::string StorageRouter::MakeKey(std::string_view exchange, std::string_view type,
                                   std::string_view symbol) {
  std::string key;
  key.reserve(exchange.size() + type.size() + symbol.size() + 3);
  key += exchange;
  key += '/';
  key += type;
  key += '/';
  key += symbol;
  return key;
}

StorageRouter::StorageRouter(const StorageConfig& storage_cfg,
                             const std::vector<ExchangeConfig>& exchanges)
    : use_engine_(storage_cfg.use_engine),
      buffer_size_(storage_cfg.dolphindb.buffer_size),
      csv_output_path_(storage_cfg.csv.output_path),
      mmap_output_path_(storage_cfg.mmap.output_path),
      buffer_a_(),
      buffer_b_() {
  buffer_a_.reserve(buffer_size_);
  buffer_b_.reserve(buffer_size_);

  if (use_engine_ == "dolphindb")
    dolphindb_.Connect(storage_cfg.dolphindb.host, storage_cfg.dolphindb.port,
                       storage_cfg.dolphindb.user, storage_cfg.dolphindb.password);

  // Pre-allocate all writers at startup — runtime maps are read-only, no lock needed
  for (const auto& ex : exchanges) {
    if (!ex.enabled) continue;
    for (const auto& ch : ex.channels) {
      for (const auto& sym : ch.symbols) {
        if (!sym.enabled) continue;
        auto key = MakeKey(ex.name, ch.type, sym.name);

        if (use_engine_ == "csv") {
          CsvWriter w;
          if (w.Open(csv_output_path_, ex.name, ch.type, sym.name))
            csv_writers_.emplace(key, std::move(w));
        }

        // Always init mmap engines (used as fallback for dolphindb too)
        {
          auto mmap_dir = mmap_output_path_;
          if (!mmap_dir.empty() && mmap_dir.back() != '/') mmap_dir += '/';
          mmap_dir += key + '/';

          auto tick_eng = std::make_unique<MmapStorageEngine>();
          if (tick_eng->OpenOrCreate(mmap_dir, "tick"))
            tick_mmap_.emplace(key, std::move(tick_eng));

          auto ob_eng = std::make_unique<MmapStorageEngine>();
          if (ob_eng->OpenOrCreate(mmap_dir, "ob"))
            ob_mmap_.emplace(key, std::move(ob_eng));
        }
      }
    }
  }
}

void StorageRouter::RouteTick(const TickData& tick, std::string_view exchange,
                               std::string_view channel_type) {
  if (use_engine_ == "csv") {
    auto key = MakeKey(exchange, channel_type, tick.symbol);
    auto it = csv_writers_.find(key);
    if (it != csv_writers_.end()) it->second.AppendTick(tick);
  } else if (use_engine_ == "mmap") {
    auto key = MakeKey(exchange, channel_type, tick.symbol);
    auto it = tick_mmap_.find(key);
    if (it != tick_mmap_.end()) it->second->AppendRecord(tick, 0);
  } else if (use_engine_ == "dolphindb") {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    auto& buf = ActiveBuffer();
    buf.push_back(tick);
    if (buf.size() >= buffer_size_) {
      FlushActiveBuffer();
      SwapBuffer();
      ActiveBuffer().clear();
    }
  }
}

void StorageRouter::RouteOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                                    uint64_t local_ts, std::string_view symbol,
                                    uint32_t depth_level, std::string_view exchange,
                                    std::string_view channel_type) {
  auto key = MakeKey(exchange, channel_type, symbol);

  if (use_engine_ == "csv") {
    auto it = csv_writers_.find(key);
    if (it != csv_writers_.end())
      it->second.AppendOrderbook(lob, exchange_ts, local_ts, symbol, depth_level);
  } else if (use_engine_ == "mmap") {
    auto it = ob_mmap_.find(key);
    if (it == ob_mmap_.end()) return;
    // F20: serialize full orderbook snapshot
    PriceLevel bids[kMaxOrderbookLevels];
    PriceLevel asks[kMaxOrderbookLevels];
    uint32_t bid_count = lob.TopBids(bids, depth_level);
    uint32_t ask_count = lob.TopAsks(asks, depth_level);

    struct alignas(8) OrderbookRecordHeader {
      uint64_t exchange_timestamp;
      uint64_t local_timestamp;
      char symbol[12];
      uint32_t bid_count;
      uint32_t ask_count;
    };
    OrderbookRecordHeader hdr{};
    hdr.exchange_timestamp = exchange_ts;
    hdr.local_timestamp = local_ts;
    std::strncpy(hdr.symbol, symbol.data(), sizeof(hdr.symbol) - 1);
    hdr.bid_count = bid_count;
    hdr.ask_count = ask_count;

    it->second->AppendRaw(&hdr, sizeof(hdr));
    if (bid_count > 0) it->second->AppendRaw(bids, bid_count * sizeof(PriceLevel));
    if (ask_count > 0) it->second->AppendRaw(asks, ask_count * sizeof(PriceLevel));
  }
}

void StorageRouter::FlushActiveBuffer() {
  auto& buf = ActiveBuffer();
  if (buf.empty()) return;

  if (!degraded_ && dolphindb_.IsHealthy()) {
    bool ok = dolphindb_.TableInsert("trades", buf);
    if (!ok) {
      LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB insert failed, degrading to mmap");
      degraded_ = true;
    }
  }

  if (degraded_) {
    for (const auto& tick : buf) {
      // Degraded fallback: write to first available mmap engine
      if (!tick_mmap_.empty())
        tick_mmap_.begin()->second->AppendRecord(tick, 1);
    }
  }
}

void StorageRouter::FlushAndClose() {
  {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    FlushActiveBuffer();
  }

  for (auto& [k, w] : csv_writers_) w.Close();
  for (auto& [k, e] : tick_mmap_) { e->Sync(); e->Close(); }
  for (auto& [k, e] : ob_mmap_) { e->Sync(); e->Close(); }

  dolphindb_.Disconnect();
}

std::vector<TickData>& StorageRouter::ActiveBuffer() {
  return active_index_.load(std::memory_order_acquire) == 0 ? buffer_a_ : buffer_b_;
}

void StorageRouter::SwapBuffer() {
  size_t cur = active_index_.load(std::memory_order_relaxed);
  active_index_.store(cur ^ 1, std::memory_order_release);
}

}  // namespace sqc
