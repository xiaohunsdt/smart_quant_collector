#include "storage_router.h"

#include <cstring>

#include "config/config_loader.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

std::string StorageRouter::MakeKey(std::string_view exchange, ChannelType type, std::string_view symbol) {
  auto name = ChannelTypeName(type);
  size_t name_len = std::strlen(name);
  std::string key;
  key.reserve(exchange.size() + name_len + symbol.size() + 3);
  key += exchange;
  key += '/';
  key += name;
  key += '/';
  key += symbol;
  return key;
}

StorageRouter::StorageRouter()
    : use_engine_(Config::Instance().storage.use_engine),
      buffer_size_(Config::Instance().storage.dolphindb.buffer_size),
      csv_output_path_(Config::Instance().storage.csv.output_path),
      mmap_output_path_(Config::Instance().storage.mmap.output_path),
      buffer_a_(),
      buffer_b_() {
  buffer_a_.reserve(buffer_size_);
  buffer_b_.reserve(buffer_size_);

  if (use_engine_ == "dolphindb")
    dolphindb_.Connect(Config::Instance().storage.dolphindb.host, Config::Instance().storage.dolphindb.port, Config::Instance().storage.dolphindb.user, Config::Instance().storage.dolphindb.password);

  for (const auto& ex : Config::Instance().exchanges) {
    if (!ex.enabled) continue;
    for (const auto& ch : ex.channels) {
      for (const auto& sym : ch.symbols) {
        if (!sym.enabled) continue;
        if (!Config::Instance().storage.persist_to_disk || !sym.persist_to_disk) continue;
        auto key = MakeKey(ex.name, ParseChannelType(ch.type), sym.name);

        if (use_engine_ == "csv") {
          CsvWriter w;
          if (w.Open(csv_output_path_, ex.name, ch.type, sym.name))
            csv_writers_.emplace(key, std::move(w));
        }

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

void StorageRouter::RouteTick(const TickData& tick, const ChannelInfo& info) {
  if (use_engine_ == "csv") {
    auto key = MakeKey(info.exchange, info.type, tick.symbol);
    auto it = csv_writers_.find(key);
    if (it != csv_writers_.end()) it->second.AppendTick(tick);
  } else if (use_engine_ == "mmap") {
    auto key = MakeKey(info.exchange, info.type, tick.symbol);
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

void StorageRouter::RouteOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, const ChannelInfo& info) {
  auto key = MakeKey(info.exchange, info.type, event.symbol);

  if (use_engine_ == "csv") {
    auto it = csv_writers_.find(key);
    if (it != csv_writers_.end())
      it->second.AppendOrderbook(event, local_ts, info.depth_level);
  } else if (use_engine_ == "mmap") {
    auto it = ob_mmap_.find(key);
    if (it == ob_mmap_.end()) return;

    struct alignas(8) OrderbookRecordHeader {
      uint64_t exchange_timestamp;
      uint64_t local_timestamp;
      char symbol[12];
      uint32_t bid_count;
      uint32_t ask_count;
    };
    OrderbookRecordHeader hdr{};
    hdr.exchange_timestamp = event.exchange_timestamp;
    hdr.local_timestamp = local_ts;
    std::strncpy(hdr.symbol, event.symbol, sizeof(hdr.symbol) - 1);
    hdr.bid_count = event.bid_count;
    hdr.ask_count = event.ask_count;

    it->second->AppendRaw(&hdr, sizeof(hdr));
    if (event.bid_count > 0)
      it->second->AppendRaw(event.bids, event.bid_count * sizeof(PriceLevel));
    if (event.ask_count > 0)
      it->second->AppendRaw(event.asks, event.ask_count * sizeof(PriceLevel));
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
      if (!tick_mmap_.empty())
        tick_mmap_.begin()->second->AppendRecord(tick, 1);
    }
  }
}

void StorageRouter::RouteBookTicker(const BookTickerEvent& event, const ChannelInfo& info) {
  if (use_engine_ == "csv") {
    auto key = MakeKey(info.exchange, info.type, event.symbol);
    auto it = csv_writers_.find(key);
    if (it != csv_writers_.end()) it->second.AppendBookTicker(event);
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
