#include "storage/backend/dolphindb_backend.h"

#include <chrono>
#include <shared_mutex>
#include <thread>

#include "common/logger_init.h"
#include "common/signal_handler.h"
#include "common/spin_hint.h"
#include "quill/LogMacros.h"
#include "storage/timestamp_util.h"

namespace sqc {

DolphinDBBackend::~DolphinDBBackend() {
  // Best-effort drain on abnormal exit; FlushAndClose() is the normal path.
  try {
    FlushAndClose();
  } catch (...) {  // NOLINT(bugprone-empty-catch)
  }
}

void DolphinDBBackend::Configure(const std::string& host, uint16_t port, const std::string& user, const std::string& password,
                                 uint32_t buffer_size, uint64_t flush_interval_ns, const std::string& mmap_output_path) {
  host_ = host;
  port_ = port;
  user_ = user;
  // Take ownership of the plaintext into a SecureString that wipes itself on
  // destruction. The `std::string(password)` copy is moved into SecureString and
  // its leftover bytes are OPENSSL_cleanse'd by FromPlain before it goes out of
  // scope — so the plaintext is not retained on the heap as it would be if
  // stored as a plain std::string member for the backend's lifetime.
  password_ = SecureString::FromPlain(std::string(password));
  buffer_size_ = buffer_size;
  flush_interval_ns_ = flush_interval_ns;
  mmap_output_path_ = mmap_output_path;
  // Pre-reserve both double-buffer slots so the hot path never allocates.
  tick_buffer_a_.reserve(buffer_size_);
  tick_buffer_b_.reserve(buffer_size_);

  if (!dolphindb_.Connect(host_, port_, user_, password_.get())) {
    LOG_ERROR(GetLogger(), "DolphinDBBackend: initial Connect failed to {}:{}", host_, port_);
  }
}

void DolphinDBBackend::RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                                       uint32_t depth_level) {
  dolphindb_.RegisterChannel(channel_id, std::string(exchange), std::string(market_type), std::string(symbol), depth_level);
}

void DolphinDBBackend::FreezeChannels() { dolphindb_.FreezeChannels(); }

// ============================================================
// InsertWithFallback — shared degrade→reconnect→retry→fallback sequence
// ============================================================

template <typename InsertFn, typename FallbackFn>
bool DolphinDBBackend::InsertWithFallback(InsertFn try_insert, FallbackFn fallback, bool retry_after_reconnect) {
  // Phase 1: attempt insert under shared lock while not degraded.
  {
    std::shared_lock lock(dolphindb_mtx_);
    if (!degraded_.load(std::memory_order_relaxed)) {
      try {
        if (try_insert()) return true;
      } catch (const std::exception& e) {
        LOG_ERROR(GetLogger(), "DolphinDBBackend: insert threw: {}", e.what());
      }
      LOG_WARNING(GetLogger(), "DolphinDBBackend: insert failed, degrading");
      degraded_.store(true, std::memory_order_relaxed);
    }
  }
  // Phase 2: degraded — attempt recovery, optionally retry once.
  if (TryReconnect()) {
    if (retry_after_reconnect) {
      std::shared_lock lock(dolphindb_mtx_);
      if (!degraded_.load(std::memory_order_relaxed)) {
        try {
          if (try_insert()) return true;
        } catch (const std::exception& e) {
          LOG_ERROR(GetLogger(), "DolphinDBBackend: retry insert threw: {}", e.what());
        }
        LOG_WARNING(GetLogger(), "DolphinDBBackend: retry insert failed, re-degrading to mmap");
        degraded_.store(true, std::memory_order_relaxed);
      }
    }
  }
  // Phase 3: write through the mmap fallback.
  fallback();
  return false;
}

// ============================================================
// TryReconnect — CAS-gated, exclusive-locked recovery attempt
// ============================================================
//
// Only one thread executes the actual Reconnect(). Other threads spin-wait
// until the recovery attempt completes, then re-read the degraded flag.
//
// IMPORTANT: caller must NOT hold dolphindb_mtx_ (shared or exclusive) when
// calling this — TryReconnect acquires an exclusive lock internally, and a
// shared→exclusive upgrade would deadlock.

bool DolphinDBBackend::TryReconnect() {
  bool expected = false;
  if (!reconnecting_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
    // Another thread is already reconnecting — spin-wait for completion.
    int spin_count = 0;
    while (reconnecting_.load(std::memory_order_acquire)) {
      if (SignalHandler::IsShutdownRequested()) return false;
      if (++spin_count > kReconnectMaxSpin) return false;  // timeout — let caller degrade
      if (spin_count > kReconnectSleepThreshold)
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      else
        SpinHint();
    }
    return !degraded_.load(std::memory_order_acquire);
  }

  std::unique_lock lock(dolphindb_mtx_);
  const bool ok = dolphindb_.Reconnect();
  if (ok) {
    LOG_INFO(GetLogger(), "DolphinDBBackend: recovered, switching back from mmap degradation");
    degraded_.store(false, std::memory_order_release);
  }
  reconnecting_.store(false, std::memory_order_release);
  return ok;
}

// ============================================================
// Mmap fallback accessors
// ============================================================

static MmapBackend& FallbackRef(std::unique_ptr<MmapBackend>& fb, std::once_flag& once, const std::string& mmap_output_path) {
  // call_once guarantees exactly one creation across concurrent callers; after
  // it returns, fb is non-null and stable, so the fast `if (fb) return *fb`
  // path below is a safe plain read (the once_flag's internal sync published it).
  std::call_once(once, [&] {
    fb = std::make_unique<MmapBackend>();
    fb->SetOutputRoot(mmap_output_path);
    fb->EnsureFallback();
  });
  return *fb;
}

// ============================================================
// InsertTick — buffer batches into MTW
// ============================================================

void DolphinDBBackend::InsertTick(const TickData& tick) {
  std::vector<TickData> to_flush;
  {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    std::vector<TickData>& active = active_buffer_ == 0 ? tick_buffer_a_ : tick_buffer_b_;
    active.push_back(tick);

    const bool threshold_reached = active.size() >= buffer_size_;
    bool time_elapsed = false;
    if (!threshold_reached && !active.empty()) {
      const uint64_t now_ns =
          std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
      if (now_ns - last_flush_ns_ > flush_interval_ns_) {
        time_elapsed = true;
      }
    }

    if (threshold_reached || time_elapsed) {
      // Move the full buffer's data out (leaving the buffer empty), then flip to
      // the other pre-reserved buffer for subsequent pushes. std::vector move
      // leaves the source's capacity implementation-defined, so defensively
      // re-reserve if it was shrunk — this is a no-op on libstdc++/libc++ (which
      // preserve capacity), guaranteeing zero hot-path allocation regardless.
      to_flush = std::move(active);
      if (active.capacity() < buffer_size_) active.reserve(buffer_size_);
      active_buffer_ ^= 1;
      last_flush_ns_ = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
  }
  if (!to_flush.empty()) {
    FlushBuffer(std::move(to_flush));
  }
}

void DolphinDBBackend::InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t /*depth_level*/) {
  if (!timestamp_util::IsValidExchangeTimestamp(event.exchange_timestamp)) {
    LOG_WARNING(GetLogger(), "DolphinDBBackend: dropping orderbook with invalid exchange_timestamp={} symbol={}", event.exchange_timestamp,
                event.symbol);
    return;
  }
  InsertWithFallback([&] { return dolphindb_.TableInsertOrderbook(event, local_ts); },
                     [&] { FallbackRef(fallback_, fallback_once_, mmap_output_path_).AppendFallbackOrderbook(event, local_ts); },
                     /*retry_after_reconnect=*/true);
}

void DolphinDBBackend::InsertBookTicker(const BookTickerEvent& event) {
  if (!timestamp_util::IsValidExchangeTimestamp(event.exchange_timestamp)) {
    LOG_WARNING(GetLogger(), "DolphinDBBackend: dropping bookticker with invalid exchange_timestamp={} symbol={}", event.exchange_timestamp,
                event.symbol);
    return;
  }
  InsertWithFallback([&] { return dolphindb_.TableInsertBookTicker(event); },
                     [&] { FallbackRef(fallback_, fallback_once_, mmap_output_path_).AppendFallbackBookTicker(event); },
                     /*retry_after_reconnect=*/true);
}

void DolphinDBBackend::FlushBuffer(std::vector<TickData>&& batch) {
  if (batch.empty()) return;

  std::vector<TickData> valid_batch;
  valid_batch.reserve(batch.size());
  for (const auto& tick : batch) {
    if (timestamp_util::IsValidExchangeTimestamp(tick.exchange_timestamp)) {
      valid_batch.push_back(tick);
    } else {
      LOG_WARNING(GetLogger(), "DolphinDBBackend: dropping trade with invalid exchange_timestamp={} symbol={}", tick.exchange_timestamp,
                  tick.symbol);
    }
  }
  if (valid_batch.empty()) return;

  // Trades never retry through MTW after a reconnect: DestroyWriters (called in
  // TryReconnect -> Reconnect) may have already flushed partially-committed
  // rows via waitForThreadCompletion(), so re-inserting the full batch would
  // duplicate rows 0..N-1. The fallback writes the whole batch to mmap instead.
  auto write_batch_to_mmap = [&valid_batch, this]() {
    MmapBackend& fb = FallbackRef(fallback_, fallback_once_, mmap_output_path_);
    for (const auto& tick : valid_batch) fb.AppendFallbackTick(tick);
  };
  InsertWithFallback([&] { return dolphindb_.TableInsertTrades(valid_batch); }, write_batch_to_mmap, /*retry_after_reconnect=*/false);
}

void DolphinDBBackend::FlushAndClose() {
  // Drain both tick double-buffer slots (OB/BT go straight to MTW, no buffer).
  // Move them out from under the lock, then flush outside it so FlushBuffer can
  // take dolphindb_mtx_ without nesting it under buffer_mtx_.
  std::vector<TickData> drain_a, drain_b;
  {
    std::lock_guard<std::mutex> lock(buffer_mtx_);
    drain_a = std::move(tick_buffer_a_);
    drain_b = std::move(tick_buffer_b_);
  }
  if (!drain_a.empty()) FlushBuffer(std::move(drain_a));
  if (!drain_b.empty()) FlushBuffer(std::move(drain_b));

  if (fallback_) {
    fallback_->FlushAndClose();
    // Release the MmapBackend now so its destructor (~MmapBackend -> FlushAndClose
    // -> per-engine Sync/Close) doesn't run a redundant second pass on engines we
    // just flushed. MmapBackend::Close/Close are idempotent, but the double sync
    // is wasted I/O; reset() makes the ownership handoff explicit.
    fallback_.reset();
  }
  dolphindb_.Disconnect();
}

}  // namespace sqc
