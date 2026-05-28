#pragma once

#include <atomic>
#include <chrono>
#include <functional>

#include <boost/circular_buffer.hpp>

#include "orderbook_event.h"

namespace sqc {

class LocalLOB;

// Lock-Step state machine for orderbook recovery, per spec §6
enum class SyncState { ACTIVE, SYNCING };

class OrderbookStateMachine {
 public:
  using SnapshotFetchCb = std::function<void()>;

  explicit OrderbookStateMachine(LocalLOB& lob, bool snapshot_mode = false);

  void OnDepthEventReceived(const DepthUpdateEvent& event);

  // Called by bg snapshot thread to post a fetched snapshot.
  // The parser thread will consume it on the next depth event.
  void PostSnapshot(const OrderbookSnapshot& snapshot, uint64_t last_id);

  // Parser-thread-only: apply a snapshot + replay buffer.
  // Kept public for tests; production use via PostSnapshot.
  void OnSnapshotReturned(uint64_t snapshot_last_id, const OrderbookSnapshot& snapshot);

  SyncState state() const { return state_.load(std::memory_order_relaxed); }
  uint32_t sync_retry_count() const { return sync_retry_count_.load(std::memory_order_relaxed); }

  void SetSnapshotFetchCb(SnapshotFetchCb cb) { snapshot_fetch_cb_ = std::move(cb); }

  // For testing: inject custom time
  void set_now(std::chrono::steady_clock::time_point now) {
    now_ = now;
    use_fake_clock_ = true;
  }

 private:
  void RequestHTTPSnapshot();
  void ResetSyncing();
  void ApplyPendingSnapshot();

  std::atomic<SyncState> state_{SyncState::ACTIVE};
  LocalLOB& lob_;
  std::atomic<uint64_t> last_update_id_{0};
  std::atomic<uint32_t> sync_retry_count_{0};
  std::chrono::steady_clock::time_point snapshot_request_time_;
  boost::circular_buffer<DepthUpdateEvent> ring_buffer_{10000};
  bool snapshot_mode_ = false;
  SnapshotFetchCb snapshot_fetch_cb_;

  // Pending snapshot from bg thread
  OrderbookSnapshot pending_snapshot_{};
  uint64_t pending_snapshot_last_id_ = 0;
  std::atomic<bool> pending_snapshot_ready_{false};

  // Test-injectable clock
  std::chrono::steady_clock::time_point now_{};
  bool use_fake_clock_ = false;
};

}  // namespace sqc
