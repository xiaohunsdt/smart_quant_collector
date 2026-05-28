#include "lockstep_fsm.h"

#include "local_lob.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

OrderbookStateMachine::OrderbookStateMachine(LocalLOB& lob, bool snapshot_mode)
    : lob_(lob), snapshot_mode_(snapshot_mode) {}

void OrderbookStateMachine::OnDepthEventReceived(const DepthUpdateEvent& event) {
  // Check for pending snapshot from bg thread before processing
  if (pending_snapshot_ready_.load(std::memory_order_acquire)) {
    pending_snapshot_ready_.store(false, std::memory_order_relaxed);
    if (state_.load(std::memory_order_relaxed) == SyncState::SYNCING) {
      ApplyPendingSnapshot();
    }
  }

  // Snapshot mode (e.g. Gate.io futures.order_book): each message is a full
  // orderbook snapshot — no incremental diff, no lockstep needed.
  if (snapshot_mode_) {
    lob_.ForceAlignWithEvent(event);
    last_update_id_.store(event.u, std::memory_order_relaxed);
    state_.store(SyncState::ACTIVE, std::memory_order_relaxed);
    return;
  }

  auto st = state_.load(std::memory_order_relaxed);
  if (st == SyncState::SYNCING) {
    // Defensive check 1: 3-second timeout or ring buffer overflow → retry
    auto now = use_fake_clock_ ? now_ : std::chrono::steady_clock::now();
    bool timed_out =
        (now - snapshot_request_time_ > std::chrono::seconds(3));
    bool overflow = ring_buffer_.full();

    if (timed_out || overflow) {
      auto retries = sync_retry_count_.load(std::memory_order_relaxed);
      retries++;
      sync_retry_count_.store(retries, std::memory_order_relaxed);
      if (retries >= 3) {
        // Defensive check 2: 3 consecutive failures → force align
        LOG_WARNING(GetLogger(),
                    "LockStep FSM: 3 consecutive sync failures, force aligning");
        state_.store(SyncState::ACTIVE, std::memory_order_relaxed);
        sync_retry_count_.store(0, std::memory_order_relaxed);
        lob_.ForceAlignWithEvent(event);
        ring_buffer_.clear();
        return;
      }
      LOG_WARNING(GetLogger(),
                  "LockStep FSM: sync retry {}/3 (timeout={}, overflow={})",
                  retries, timed_out, overflow);
      ResetSyncing();
      return;
    }
    ring_buffer_.push_back(event);  // buffer events while syncing
  } else {
    // ACTIVE: validate update_id continuity before applying
    auto last_id = last_update_id_.load(std::memory_order_relaxed);
    if (last_id != 0 && event.U != last_id + 1) {
      LOG_WARNING(GetLogger(),
                  "LockStep FSM: gap detected (expected={}, got={}), entering SYNCING",
                  last_id + 1, event.U);
      ResetSyncing();
      ring_buffer_.push_back(event);
      return;
    }
    lob_.UpdateDepth(event);
    last_update_id_.store(event.u, std::memory_order_relaxed);
  }
}

void OrderbookStateMachine::PostSnapshot(const OrderbookSnapshot& snapshot,
                                          uint64_t last_id) {
  pending_snapshot_ = snapshot;
  pending_snapshot_last_id_ = last_id;
  pending_snapshot_ready_.store(true, std::memory_order_release);
}

void OrderbookStateMachine::OnSnapshotReturned(uint64_t snapshot_last_id,
                                               const OrderbookSnapshot& snapshot) {
  if (state_.load(std::memory_order_relaxed) != SyncState::SYNCING) return;

  lob_.ApplySnapshot(snapshot);
  uint64_t current_id = snapshot_last_id;

  // Lock-step replay from ring buffer
  for (const auto& next_event : ring_buffer_) {
    if (next_event.u <= current_id) continue;
    if (next_event.U <= current_id + 1 && next_event.u >= current_id + 1) {
      lob_.UpdateDepth(next_event);
      current_id = next_event.u;
    }
  }

  last_update_id_.store(current_id, std::memory_order_relaxed);
  state_.store(SyncState::ACTIVE, std::memory_order_relaxed);
  sync_retry_count_.store(0, std::memory_order_relaxed);
  ring_buffer_.clear();
  LOG_INFO(GetLogger(),
           "LockStep FSM: sync complete, last_update_id={}", current_id);
}

void OrderbookStateMachine::ApplyPendingSnapshot() {
  OnSnapshotReturned(pending_snapshot_last_id_, pending_snapshot_);
}

void OrderbookStateMachine::RequestHTTPSnapshot() {
  snapshot_request_time_ =
      use_fake_clock_ ? now_ : std::chrono::steady_clock::now();
  LOG_INFO(GetLogger(), "LockStep FSM: requesting HTTP snapshot");
  if (snapshot_fetch_cb_) {
    snapshot_fetch_cb_();
  }
}

void OrderbookStateMachine::ResetSyncing() {
  ring_buffer_.clear();
  state_.store(SyncState::SYNCING, std::memory_order_relaxed);
  RequestHTTPSnapshot();
}

}  // namespace sqc
