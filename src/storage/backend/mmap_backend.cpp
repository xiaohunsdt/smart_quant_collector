#include "storage/backend/mmap_backend.h"

#include "common/logger_init.h"
#include "common/path_util.h"
#include "quill/LogMacros.h"

namespace sqc {

MmapBackend::~MmapBackend() { FlushAndClose(); }

void MmapBackend::RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                                  uint32_t /*depth_level*/) {
  std::string key;
  key.append(exchange);
  key += '/';
  key.append(market_type);
  key += '/';
  key.append(symbol);
  const std::string mmap_dir = JoinPath(EnsureTrailingSlash(output_root_), key + "/");

  std::lock_guard<std::mutex> lock(mtx_);
  auto tick_eng = std::make_unique<MmapStorageEngine>();
  if (tick_eng->OpenOrCreate(mmap_dir, "tick")) tick_engines_.emplace(channel_id, std::move(tick_eng));

  auto ob_eng = std::make_unique<MmapStorageEngine>();
  if (ob_eng->OpenOrCreate(mmap_dir, "ob")) ob_engines_.emplace(channel_id, std::move(ob_eng));

  auto bt_eng = std::make_unique<MmapStorageEngine>();
  if (bt_eng->OpenOrCreate(mmap_dir, "bt")) bt_engines_.emplace(channel_id, std::move(bt_eng));
}

void MmapBackend::InsertTick(const TickData& tick) {
  // Map is frozen after FreezeChannels() — concurrent find is data-race-free.
  // Look up without the lock, then lock only for the append.
  auto it = tick_engines_.find(tick.channel_id);
  if (it == tick_engines_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  it->second->AppendRecord(tick, 0);
}

void MmapBackend::InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t /*depth_level*/) {
  auto it = ob_engines_.find(event.channel_id);
  if (it == ob_engines_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  WriteOrderbook(*it->second, event, local_ts);
}

void MmapBackend::InsertBookTicker(const BookTickerEvent& event) {
  auto it = bt_engines_.find(event.channel_id);
  if (it == bt_engines_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  WriteBookTicker(*it->second, event);
}

void MmapBackend::FlushAndClose() {
  std::lock_guard<std::mutex> lock(mtx_);
  for (auto& [k, e] : tick_engines_) {
    e->Sync();
    e->Close();
  }
  for (auto& [k, e] : ob_engines_) {
    e->Sync();
    e->Close();
  }
  for (auto& [k, e] : bt_engines_) {
    e->Sync();
    e->Close();
  }
  if (fallback_) {
    fallback_->Sync();
    fallback_->Close();
    fallback_.reset();
  }
}

void MmapBackend::EnsureFallback() {
  std::lock_guard<std::mutex> lock(mtx_);
  if (fallback_) return;
  auto fb = std::make_unique<MmapStorageEngine>();
  const std::string fallback_path = JoinPath(EnsureTrailingSlash(output_root_), "degraded/");
  if (!fb->OpenOrCreate(fallback_path, "tick")) {
    LOG_ERROR(GetLogger(), "MmapBackend: failed to create fallback mmap for degradation");
  } else {
    fallback_ = std::move(fb);
  }
}

void MmapBackend::AppendFallbackTick(const TickData& tick) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (fallback_) fallback_->AppendRecord(tick, 1);
}

void MmapBackend::AppendFallbackOrderbook(const DepthUpdateEvent& event, uint64_t local_ts) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (fallback_) WriteOrderbook(*fallback_, event, local_ts);
}

void MmapBackend::AppendFallbackBookTicker(const BookTickerEvent& event) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (fallback_) WriteBookTicker(*fallback_, event);
}

// ============================================================
// Serialization helpers — single source for the binary layout
// ============================================================

void MmapBackend::WriteOrderbook(MmapStorageEngine& eng, const DepthUpdateEvent& event, uint64_t local_ts) {
  record_layout::OrderbookRecordHeader hdr{};
  hdr.exchange_timestamp = event.exchange_timestamp;
  hdr.local_diff = local_ts;
  record_layout::CopySymbol(hdr.symbol, event.symbol, sizeof(event.symbol));
  hdr.bid_count = event.bid_count;
  hdr.ask_count = event.ask_count;

  eng.AppendRaw(&hdr, sizeof(hdr));
  if (event.bid_count > 0) eng.AppendRaw(event.bids, event.bid_count * sizeof(PriceLevel));
  if (event.ask_count > 0) eng.AppendRaw(event.asks, event.ask_count * sizeof(PriceLevel));
}

void MmapBackend::WriteBookTicker(MmapStorageEngine& eng, const BookTickerEvent& event) {
  record_layout::BookTickerRecord rec{};
  rec.exchange_timestamp = event.exchange_timestamp;
  rec.local_diff = event.local_diff;
  record_layout::CopySymbol(rec.symbol, event.symbol, sizeof(event.symbol));
  rec.best_bid_price = event.best_bid_price;
  rec.best_bid_qty = event.best_bid_qty;
  rec.best_ask_price = event.best_ask_price;
  rec.best_ask_qty = event.best_ask_qty;

  eng.AppendRaw(&rec, sizeof(rec));
}

}  // namespace sqc
