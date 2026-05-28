#pragma once

#include <chrono>

#include <boost/circular_buffer.hpp>

#include "orderbook_event.h"

namespace sqc {

class LocalLOB;

// Lock-Step state machine for orderbook recovery, per spec §6
enum class SyncState { ACTIVE, SYNCING };

class OrderbookStateMachine {
 public:
  explicit OrderbookStateMachine(LocalLOB& lob, bool snapshot_mode = false);

  void OnDepthEventReceived(const DepthUpdateEvent& event);
  void OnSnapshotReturned(uint64_t snapshot_last_id, const OrderbookSnapshot& snapshot);

  SyncState state() const { return state_; }
  uint32_t sync_retry_count() const { return sync_retry_count_; }

  // For testing: inject custom time
  void set_now(std::chrono::steady_clock::time_point now) {
    now_ = now;
    use_fake_clock_ = true;
  }

 private:
  void RequestHTTPSnapshot();
  void ResetSyncing();

  SyncState state_ = SyncState::ACTIVE;
  LocalLOB& lob_;
  uint64_t last_update_id_ = 0;
  uint32_t sync_retry_count_ = 0;
  std::chrono::steady_clock::time_point snapshot_request_time_;
  boost::circular_buffer<DepthUpdateEvent> ring_buffer_{10000};
  bool snapshot_mode_ = false;

  // Test-injectable clock
  std::chrono::steady_clock::time_point now_{};
  bool use_fake_clock_ = false;
};

}  // namespace sqc
