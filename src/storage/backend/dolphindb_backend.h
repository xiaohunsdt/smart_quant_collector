#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "src/common/tick_data.h"
#include "src/config/secure_string.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/channel_registry.h"
#include "storage/client/dolphindb_client.h"
#include "storage/i_storage_backend.h"
#include "storage/backend/mmap_backend.h"

namespace sqc {

/// DolphinDB IStorageBackend. Owns the DolphinDBClient connection, the tick
/// accumulation buffer, the degradation/reconnect state machine, and the mmap
/// fallback sink. All of the degrade→reconnect→retry→fallback logic that used
/// to live in StorageRouter is centralized here (it only ever applied to the
/// DolphinDB path), leaving the router as a pure facade.
///
/// Threading: hot-path inserts take a shared_lock on dolphindb_mtx_;
/// Reconnect/Disconnect take a unique_lock. The tick buffer is guarded by
/// buffer_mtx_. Same contract the router exposed previously.
class DolphinDBBackend : public IStorageBackend {
 public:
  DolphinDBBackend() = default;
  ~DolphinDBBackend() override;
  DolphinDBBackend(const DolphinDBBackend&) = delete;
  DolphinDBBackend& operator=(const DolphinDBBackend&) = delete;
  DolphinDBBackend(DolphinDBBackend&&) = delete;
  DolphinDBBackend& operator=(DolphinDBBackend&&) = delete;

  // --- IStorageBackend ---
  void RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                       uint32_t depth_level) override;
  void FreezeChannels() override;
  void InsertTick(const TickData& tick) override;
  void InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level) override;
  void InsertBookTicker(const BookTickerEvent& event) override;
  void FlushAndClose() override;
  const char* Name() const noexcept override { return "dolphindb"; }

  // Wired by the factory from the config snapshot before RegisterChannel.
  void Configure(const std::string& host, uint16_t port, const std::string& user, const std::string& password, uint32_t buffer_size,
                 uint64_t flush_interval_ns, const std::string& mmap_output_path);

 private:
  void FlushBuffer(std::vector<TickData>&& batch);

  // Shared degrade→reconnect→(optional retry)→fallback sequence. `try_insert`
  // runs under a shared lock on dolphindb_mtx_ and returns true on success.
  // When retry_after_reconnect is false, a successful reconnect skips retrying
  // MTW (trades: a partial flush during reconnect could duplicate rows) and
  // goes straight to fallback. Returns true if data was written to DolphinDB.
  template <typename InsertFn, typename FallbackFn>
  bool InsertWithFallback(InsertFn try_insert, FallbackFn fallback, bool retry_after_reconnect);

  bool TryReconnect();

  // Reconnect spin-loop tuning (named, was inline magic literals).
  static constexpr int kReconnectMaxSpin = 10000;
  static constexpr int kReconnectSleepThreshold = 1000;

  std::string host_;
  uint16_t port_ = 0;
  std::string user_;
  SecureString password_;  // wiped on destruction; was a plain std::string (survived in core dumps)
  uint32_t buffer_size_ = 2000;
  uint64_t flush_interval_ns_ = 10'000'000ULL;

  DolphinDBClient dolphindb_;
  std::shared_mutex dolphindb_mtx_;  // shared: hot-path inserts; unique: reconnect
  // SYNCHRONIZATION CONTRACT: degraded_ is read/written with memory_order_relaxed
  // in InsertWithFallback because the surrounding shared_lock on dolphindb_mtx_
  // (reads) and the unique_lock in TryReconnect (the single writer that stores
  // false) provide the required happens-before edges. Do NOT drop the locks or
  // access degraded_ from an unlocked path without promoting the ordering to
  // acquire/release — doing so would introduce a data race.
  std::atomic<bool> degraded_{false};
  std::atomic<bool> reconnecting_{false};  // CAS gate — only one thread runs Reconnect()

  // Fallback sink (shared with the mmap engine selection when present).
  // Lazily constructed on first degradation. fallback_once_ serializes the
  // first-time creation: multiple parser threads may hit the fallback path
  // concurrently after DolphinDB degrades, and an unsynchronized check-then-
  // assign on a unique_ptr is a data race (the losing thread's MmapBackend is
  // destroyed mid-use and its data lost). Subsequent appends are serialized by
  // MmapBackend's own mtx_.
  std::unique_ptr<MmapBackend> fallback_;
  std::once_flag fallback_once_;
  std::string mmap_output_path_;

  // Tick double-buffer: both pre-reserved at Configure() time. InsertTick
  // pushes to the active buffer under buffer_mtx_; on flush it swaps the full
  // buffer out to a local and toggles active_buffer_, so the now-active buffer
  // still has its reserved capacity — zero heap allocation on the hot path
  // after startup. (A single buffer forced a per-flush reserve under the lock.)
  std::vector<TickData> tick_buffer_a_;
  std::vector<TickData> tick_buffer_b_;
  size_t active_buffer_ = 0;
  uint64_t last_flush_ns_ = 0;
  std::mutex buffer_mtx_;
};

}  // namespace sqc
