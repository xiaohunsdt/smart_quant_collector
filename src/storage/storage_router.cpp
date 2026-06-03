#include "storage_router.h"

#include <cstring>
#include <thread>

#include "common/logger_init.h"
#include "common/signal_handler.h"
#include "config/config_loader.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace {

// Binary record header for orderbook mmap persistence.
// Both RouteOrderbook (direct mmap) and WriteOrderbookToMmap (degradation fallback) use this.
struct alignas(8) OrderbookRecordHeader {
  uint64_t exchange_timestamp;
  uint64_t local_diff;
  char symbol[32];
  uint32_t bid_count;
  uint32_t ask_count;
};

}  // namespace

StorageRouter::StorageRouter()
    : use_engine_(Config::Instance().storage.use_engine),
      buffer_size_(Config::Instance().storage.dolphindb.buffer_size),
      csv_output_path_(Config::Instance().storage.csv.output_path),
      mmap_output_path_(Config::Instance().storage.mmap.output_path) {
  buffer_a_.reserve(buffer_size_);
  buffer_b_.reserve(buffer_size_);

  if(use_engine_ == "dolphindb") {
#ifdef SQC_WITH_DOLPHINDB
    dolphindb_.Connect(Config::Instance().storage.dolphindb.host, Config::Instance().storage.dolphindb.port,
                       Config::Instance().storage.dolphindb.user, Config::Instance().storage.dolphindb.password.get());
#else
    LOG_ERROR(GetLogger(),
              "StorageRouter: storage.use_engine=dolphindb requires a Linux build "
              "(SQC_WITH_DOLPHINDB); use csv or mmap on this platform");
    degraded_.store(true, std::memory_order_relaxed);
#endif
  }
}

void StorageRouter::FreezeChannels() {
#ifdef SQC_WITH_DOLPHINDB
  dolphindb_.FreezeChannels();
#endif
}

// ============================================================
// RegisterChannel
// ============================================================

void StorageRouter::RegisterChannel(uint32_t channel_id, const ChannelInfo& info, bool persist_to_disk) {
  if(!Config::Instance().storage.persist_to_disk || !persist_to_disk) return;

#ifdef SQC_WITH_DOLPHINDB
  if(use_engine_ == "dolphindb") {
    dolphindb_.RegisterChannel(channel_id, std::string(info.exchange), ChannelTypeName(info.type), std::string(info.symbol), info.depth_level);
  }
#endif

  if(use_engine_ == "csv") {
    CsvWriter w;
    if(w.Open(csv_output_path_, info.exchange, ChannelTypeName(info.type), info.symbol)) csv_writers_.emplace(channel_id, std::move(w));
  }

  if(use_engine_ == "mmap") {
    std::string key(info.exchange);
    key += '/';
    key += ChannelTypeName(info.type);
    key += '/';
    key += info.symbol;

    auto mmap_dir = mmap_output_path_;
    if(!mmap_dir.empty() && mmap_dir.back() != '/') mmap_dir += '/';
    mmap_dir += key + '/';

    auto tick_eng = std::make_unique<MmapStorageEngine>();
    if(tick_eng->OpenOrCreate(mmap_dir, "tick")) tick_mmap_.emplace(channel_id, std::move(tick_eng));

    auto ob_eng = std::make_unique<MmapStorageEngine>();
    if(ob_eng->OpenOrCreate(mmap_dir, "ob")) ob_mmap_.emplace(channel_id, std::move(ob_eng));

    auto bt_eng = std::make_unique<MmapStorageEngine>();
    if(bt_eng->OpenOrCreate(mmap_dir, "bt")) bt_mmap_.emplace(channel_id, std::move(bt_eng));
  }
}

// ============================================================
// RouteTick — double-buffer batches into MTW
// ============================================================

void StorageRouter::RouteTick(const TickData& tick, const ChannelInfo& /*info*/) {
  if(use_engine_ == "csv") {
    auto it = csv_writers_.find(tick.channel_id);
    if(it != csv_writers_.end()) {
      std::lock_guard<std::mutex> lock(storage_mtx_);
      it->second.AppendTick(tick);
    }
  } else if(use_engine_ == "mmap") {
    auto it = tick_mmap_.find(tick.channel_id);
    if(it != tick_mmap_.end()) {
      std::lock_guard<std::mutex> lock(storage_mtx_);
      it->second->AppendRecord(tick, 0);
    }
  } else if(use_engine_ == "dolphindb") {
    std::vector<TickData> to_flush;
    {
      std::lock_guard<std::mutex> lock(buffer_mtx_);
      auto& buf = ActiveBuffer();
      buf.push_back(tick);
      if(buf.size() >= buffer_size_) {
        to_flush.swap(buf);
        SwapBuffer();
      }
    }
    if(!to_flush.empty()) {
      FlushBuffer(std::move(to_flush));
    }
  }
}

// ============================================================
// RouteOrderbook — directly into MTW (MTW is thread-safe, internally batched)
// ============================================================

void StorageRouter::RouteOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, const ChannelInfo& info) {
  if(use_engine_ == "csv") {
    auto it = csv_writers_.find(event.channel_id);
    if(it != csv_writers_.end()) {
      std::lock_guard<std::mutex> lock(storage_mtx_);
      it->second.AppendOrderbook(event, local_ts, info.depth_level);
    }
  } else if(use_engine_ == "dolphindb") {
#ifdef SQC_WITH_DOLPHINDB
    // Phase 1: attempt insert under shared lock.
    {
      std::shared_lock lock(dolphindb_mtx_);
      if(!degraded_.load(std::memory_order_relaxed)) {
        try {
          if(dolphindb_.TableInsertOrderbook(event, local_ts)) return;
        } catch(const std::exception& e) {
          LOG_ERROR(GetLogger(), "StorageRouter: DolphinDB orderbook insert threw: {}", e.what());
        }
        LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB orderbook insert failed, degrading");
        degraded_.store(true, std::memory_order_relaxed);
      }
    }
    // Phase 2: degraded — try recovery or fallback to mmap.
    if(TryReconnect()) {
      std::shared_lock lock(dolphindb_mtx_);
      if(!degraded_.load(std::memory_order_relaxed)) {
        try {
          if(dolphindb_.TableInsertOrderbook(event, local_ts)) return;
        } catch(const std::exception& e) {
          LOG_ERROR(GetLogger(), "StorageRouter: DolphinDB orderbook insert threw in retry: {}", e.what());
        }
        LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB orderbook retry insert failed, re-degrading to mmap");
        degraded_.store(true, std::memory_order_relaxed);
      }
    }
    WriteOrderbookToMmap(event, local_ts);
#endif
  } else if(use_engine_ == "mmap") {
    auto it = ob_mmap_.find(event.channel_id);
    if(it == ob_mmap_.end()) return;
    std::lock_guard<std::mutex> lock(storage_mtx_);

    OrderbookRecordHeader hdr{};
    hdr.exchange_timestamp = event.exchange_timestamp;
    hdr.local_diff = local_ts;
    std::memcpy(hdr.symbol, event.symbol, sizeof(event.symbol));
    hdr.bid_count = event.bid_count;
    hdr.ask_count = event.ask_count;

    it->second->AppendRaw(&hdr, sizeof(hdr));
    if(event.bid_count > 0) it->second->AppendRaw(event.bids, event.bid_count * sizeof(PriceLevel));
    if(event.ask_count > 0) it->second->AppendRaw(event.asks, event.ask_count * sizeof(PriceLevel));
  }
}

// ============================================================
// RouteBookTicker — directly into MTW (MTW is thread-safe, internally batched)
// ============================================================

void StorageRouter::RouteBookTicker(const BookTickerEvent& event, const ChannelInfo& /*info*/) {
  if(use_engine_ == "csv") {
    auto it = csv_writers_.find(event.channel_id);
    if(it != csv_writers_.end()) {
      std::lock_guard<std::mutex> lock(storage_mtx_);
      it->second.AppendBookTicker(event);
    }
  } else if(use_engine_ == "mmap") {
    auto it = bt_mmap_.find(event.channel_id);
    if(it == bt_mmap_.end()) return;

    struct alignas(8) BookTickerRecord {
      uint64_t exchange_timestamp;
      uint64_t local_diff;
      char symbol[32];
      double best_bid_price;
      double best_bid_qty;
      double best_ask_price;
      double best_ask_qty;
    };
    BookTickerRecord rec{};
    rec.exchange_timestamp = event.exchange_timestamp;
    rec.local_diff = event.local_diff;
    std::memcpy(rec.symbol, event.symbol, sizeof(event.symbol));
    rec.best_bid_price = event.best_bid_price;
    rec.best_bid_qty = event.best_bid_qty;
    rec.best_ask_price = event.best_ask_price;
    rec.best_ask_qty = event.best_ask_qty;

    std::lock_guard<std::mutex> lock(storage_mtx_);
    it->second->AppendRaw(&rec, sizeof(rec));
  } else if(use_engine_ == "dolphindb") {
#ifdef SQC_WITH_DOLPHINDB
    {
      std::shared_lock lock(dolphindb_mtx_);
      if(!degraded_.load(std::memory_order_relaxed)) {
        try {
          if(dolphindb_.TableInsertBookTicker(event)) return;
        } catch(const std::exception& e) {
          LOG_ERROR(GetLogger(), "StorageRouter: DolphinDB bookticker insert threw: {}", e.what());
        }
        LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB bookticker insert failed, degrading");
        degraded_.store(true, std::memory_order_relaxed);
      }
    }
    if(TryReconnect()) {
      std::shared_lock lock(dolphindb_mtx_);
      if(!degraded_.load(std::memory_order_relaxed)) {
        try {
          if(dolphindb_.TableInsertBookTicker(event)) return;
        } catch(const std::exception& e) {
          LOG_ERROR(GetLogger(), "StorageRouter: DolphinDB bookticker insert threw in retry: {}", e.what());
        }
        LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB bookticker retry insert failed, re-degrading to mmap");
        degraded_.store(true, std::memory_order_relaxed);
      }
    }
    WriteBookTickerToMmap(event);
#endif
  }
}

// ============================================================
// Tick double-buffer accessors
// ============================================================

std::vector<TickData>& StorageRouter::ActiveBuffer() { return active_index_.load(std::memory_order_acquire) == 0 ? buffer_a_ : buffer_b_; }

void StorageRouter::SwapBuffer() {
  size_t cur = active_index_.load(std::memory_order_relaxed);
  active_index_.store(cur ^ 1, std::memory_order_release);
}

// ============================================================
// Degradation helpers
// ============================================================

// ============================================================
// TryReconnect — CAS-gated, exclusive-locked recovery attempt
// ============================================================
//
// Only one thread executes the actual Reconnect(). Other threads
// spin-wait until the recovery attempt completes, then re-read
// the degraded flag.
//
// IMPORTANT: caller must NOT hold dolphindb_mtx_ (shared or exclusive)
// when calling this — TryReconnect acquires an exclusive lock internally,
// and shared→exclusive upgrade would deadlock.

bool StorageRouter::TryReconnect() {
  // CAS gate: only one thread performs the actual reconnect.
  bool expected = false;
  if(!reconnecting_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
    // Another thread is already reconnecting — spin-wait for completion.
    int spin_count = 0;
    constexpr int kMaxSpin = 10000;
    constexpr int kSleepThreshold = 1000;
    while(reconnecting_.load(std::memory_order_acquire)) {
      if(SignalHandler::IsShutdownRequested()) return false;
      if(++spin_count > kMaxSpin) return false;  // timeout — let caller degrade
      if(spin_count > kSleepThreshold)
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      else
        std::this_thread::yield();
    }
    // Re-read degraded state after reconnection attempt completes.
    return !degraded_.load(std::memory_order_acquire);
  }

  // Exclusive lock: blocks all hot-path inserts during rebuild.
  // This prevents parser threads from observing partially-constructed
  // MTW instances or calling insert() on a stale writer pointer.
  std::unique_lock lock(dolphindb_mtx_);
  bool ok = dolphindb_.Reconnect();
  if(ok) {
    LOG_INFO(GetLogger(), "StorageRouter: DolphinDB recovered, switching back from mmap degradation");
    degraded_.store(false, std::memory_order_release);
  }
  reconnecting_.store(false, std::memory_order_release);
  return ok;
}

void StorageRouter::EnsureFallbackMmap() {
  // Only called during degradation (rare path) — simple mutex is fine.
  std::lock_guard<std::mutex> lock(storage_mtx_);
  if(fallback_mmap_) return;
  auto fb = std::make_unique<MmapStorageEngine>();
  std::string fallback_path = mmap_output_path_;
  if(!fallback_path.empty() && fallback_path.back() != '/') fallback_path += '/';
  fallback_path += "degraded/";
  if(!fb->OpenOrCreate(fallback_path, "tick")) {
    LOG_ERROR(GetLogger(), "StorageRouter: failed to create fallback mmap for degradation");
  } else {
    fallback_mmap_ = std::move(fb);
  }
}

void StorageRouter::WriteOrderbookToMmap(const DepthUpdateEvent& event, uint64_t local_ts) {
  EnsureFallbackMmap();
  if(!fallback_mmap_) return;

  OrderbookRecordHeader hdr{};
  hdr.exchange_timestamp = event.exchange_timestamp;
  hdr.local_diff = local_ts;
  std::memcpy(hdr.symbol, event.symbol, sizeof(event.symbol));
  hdr.bid_count = event.bid_count;
  hdr.ask_count = event.ask_count;

  std::lock_guard<std::mutex> lock(storage_mtx_);
  fallback_mmap_->AppendRaw(&hdr, sizeof(hdr));
  if(event.bid_count > 0) fallback_mmap_->AppendRaw(event.bids, event.bid_count * sizeof(PriceLevel));
  if(event.ask_count > 0) fallback_mmap_->AppendRaw(event.asks, event.ask_count * sizeof(PriceLevel));
}

void StorageRouter::WriteBookTickerToMmap(const BookTickerEvent& event) {
  EnsureFallbackMmap();
  if(!fallback_mmap_) return;

  struct alignas(8) BookTickerRecord {
    uint64_t exchange_timestamp;
    uint64_t local_diff;
    char symbol[32];
    double best_bid_price;
    double best_bid_qty;
    double best_ask_price;
    double best_ask_qty;
  };
  BookTickerRecord rec{};
  rec.exchange_timestamp = event.exchange_timestamp;
  rec.local_diff = event.local_diff;
  std::memcpy(rec.symbol, event.symbol, sizeof(event.symbol));
  rec.best_bid_price = event.best_bid_price;
  rec.best_bid_qty = event.best_bid_qty;
  rec.best_ask_price = event.best_ask_price;
  rec.best_ask_qty = event.best_ask_qty;

  std::lock_guard<std::mutex> lock(storage_mtx_);
  fallback_mmap_->AppendRaw(&rec, sizeof(rec));
}

// ============================================================
// Flush helpers — Tick only
// ============================================================

void StorageRouter::FlushActiveBuffer() {
  auto& buf = ActiveBuffer();
  if(buf.empty()) return;
  std::vector<TickData> batch;
  batch.swap(buf);
  FlushBuffer(std::move(batch));
}

void StorageRouter::FlushBuffer(std::vector<TickData>&& batch) {
  if(batch.empty()) return;

#ifdef SQC_WITH_DOLPHINDB
  {
    std::shared_lock lock(dolphindb_mtx_);
    if(!degraded_.load(std::memory_order_relaxed)) {
      try {
        if(dolphindb_.TableInsertTrades(batch)) return;
      } catch(const std::exception& e) {
        LOG_ERROR(GetLogger(), "StorageRouter: DolphinDB trade insert threw: {}", e.what());
      }
      LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB trade insert failed, degrading to mmap");
      degraded_.store(true, std::memory_order_relaxed);
    }
  }
  if(TryReconnect()) {
    // Don't retry the batch through MTW: DestroyWriters (called in
    // TryReconnect -> Reconnect) may have already flushed partially-
    // committed rows via waitForThreadCompletion().  Retrying the full
    // batch would duplicate rows 0..N-1.  Write to mmap fallback instead.
    LOG_INFO(GetLogger(), "StorageRouter: DolphinDB reconnect succeeded — writing trade batch to mmap fallback to avoid partial-batch duplication");
    EnsureFallbackMmap();
    if(fallback_mmap_) {
      std::lock_guard<std::mutex> lock(storage_mtx_);
      for(const auto& tick : batch) fallback_mmap_->AppendRecord(tick, 1);
    }
    return;
  }
#endif

  if(degraded_.load(std::memory_order_relaxed)) {
    EnsureFallbackMmap();
    std::lock_guard<std::mutex> lock(storage_mtx_);
    if(fallback_mmap_) {
      for(const auto& tick : batch) fallback_mmap_->AppendRecord(tick, 1);
    }
  }
}

// ============================================================
// FlushAndClose — drain tick buffer, then disconnect
// ============================================================

void StorageRouter::FlushAndClose() {
  // Flush tick double-buffer (OB/BT go directly to MTW, no buffer to drain)
  {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    FlushActiveBuffer();
  }

  // Close CSV and mmap engines
  for(auto& [k, w] : csv_writers_) w.Close();
  for(auto& [k, e] : tick_mmap_) {
    e->Sync();
    e->Close();
  }
  for(auto& [k, e] : ob_mmap_) {
    e->Sync();
    e->Close();
  }
  for(auto& [k, e] : bt_mmap_) {
    e->Sync();
    e->Close();
  }
  if(fallback_mmap_) {
    fallback_mmap_->Sync();
    fallback_mmap_->Close();
  }

#ifdef SQC_WITH_DOLPHINDB
  dolphindb_.Disconnect();
#endif
}

}  // namespace sqc
