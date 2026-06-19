#pragma once

#include <atomic>
#include <cstdint>

#include "common/spin_hint.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// MpscRithmicQueue<T, Capacity> — bounded lock-free MPSC queue.
//
// Dmitry Vyukov-style design:
//   - Fixed-size array of Slot{atomic<uint64_t> sequence, T data}
//   - Multi-producer: CAS (fetch_add) on write_pos_, spin-wait for slot ready,
//     write data, publish sequence
//   - Single consumer: owns read_pos_, checks sequence == read_pos + 1
//
// Capacity must be a power of 2. Zero heap allocation after construction.
// TryPush() is non-blocking: it spins for a bounded number of iterations
// waiting for the consumer to free the reserved slot, then drops the event
// (publishes a sentinel so the queue stays free of sequence holes — the
// consumer recognizes the sentinel via channel_id==0 and skips it).
// ============================================================================

template <typename T, size_t Capacity = 8192>
class MpscRithmicQueue {
  static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

 public:
  using value_type = T;

  MpscRithmicQueue();
  ~MpscRithmicQueue() = default;

  MpscRithmicQueue(const MpscRithmicQueue&) = delete;
  MpscRithmicQueue& operator=(const MpscRithmicQueue&) = delete;

  // ---- Producer API (multi-thread safe) ----

  /// Try to push an event. Returns true on success.
  /// Returns false if the queue stayed saturated for the bounded spin budget
  /// (the event is DROPPED, dropped_count_ is incremented, and a sentinel is
  /// published so the consumer advances past the reserved slot). Non-blocking
  /// — safe to call from REngine callback threads.
  [[nodiscard]] bool TryPush(const T& event) noexcept;

  /// Push with spin-wait. Blocks the caller until space is available.
  /// Not for hot path; use TryPush on hot path.
  void Push(const T& event) noexcept;

  // ---- Consumer API (single-thread only) ----

  /// Try to pop an event. Returns true on success.
  /// Returns false if the queue is empty.
  [[nodiscard]] bool TryPop(T& out) noexcept;

  /// Pop with spin-wait. Blocks until an event is available.
  T PopBlocking() noexcept;

  // ---- Telemetry ----

  [[nodiscard]] size_t size() const noexcept {
    size_t w = write_pos_.load(std::memory_order_acquire);
    size_t r = read_pos_.load(std::memory_order_relaxed);
    return w - r;
  }

  [[nodiscard]] uint64_t dropped_count() const noexcept { return dropped_count_.load(std::memory_order_relaxed); }

  [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }

 private:
  struct Slot {
    std::atomic<uint64_t> sequence;
    T data;
  };

  static constexpr size_t kMask = Capacity - 1;

  // Producer cache line
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<uint64_t> dropped_count_{0};

  // Shared data — pre-allocated slots
  Slot slots_[Capacity];

  // Consumer cache line
  alignas(64) std::atomic<size_t> read_pos_{0};
};

// ============================================================================
// Inline implementations
// ============================================================================

template <typename T, size_t Capacity>
MpscRithmicQueue<T, Capacity>::MpscRithmicQueue() {
  for(size_t i = 0; i < Capacity; ++i) {
    slots_[i].sequence.store(i, std::memory_order_relaxed);
  }
}

template <typename T, size_t Capacity>
bool MpscRithmicQueue<T, Capacity>::TryPush(const T& event) noexcept {
  // Reserve a slot via fetch_add. write_pos_ is NEVER rolled back (fetch_sub)
  // because under multi-producer contention that creates a sequence hole the
  // consumer can never advance past, deadlocking the queue.
  size_t pos = write_pos_.fetch_add(1, std::memory_order_relaxed);
  Slot& slot = slots_[pos & kMask];

  // Fast path: the consumer has already freed this slot (sequence advanced to pos).
  if(slot.sequence.load(std::memory_order_acquire) == pos) {
    slot.data = event;
    slot.sequence.store(pos + 1, std::memory_order_release);
    return true;
  }

  // Slow path: queue saturated (consumer hasn't freed the slot yet). Bounded
  // spin — never an infinite loop, since REngine callback threads MUST stay
  // responsive or they cascade into connection loss.
  constexpr int kMaxSpin = 10000;
  int spin = 0;
  while(slot.sequence.load(std::memory_order_acquire) != pos) {
    if(++spin > kMaxSpin) {
      // Still saturated: DROP the event. We must still publish the slot
      // (sequence = pos+1) so the consumer can advance — otherwise the
      // reserved-but-unpublished slot is a permanent hole. A value-initialized
      // sentinel with channel_id==0 is written; DataDispatcher::Lookup(0)
      // returns nullptr and the event is discarded harmlessly.
      slot.data = T{};
      slot.sequence.store(pos + 1, std::memory_order_release);
      dropped_count_.fetch_add(1, std::memory_order_relaxed);
      return false;
    }
    SpinHint();
  }

  slot.data = event;
  slot.sequence.store(pos + 1, std::memory_order_release);
  return true;
}

template <typename T, size_t Capacity>
void MpscRithmicQueue<T, Capacity>::Push(const T& event) noexcept {
  size_t pos = write_pos_.fetch_add(1, std::memory_order_relaxed);
  Slot& slot = slots_[pos & kMask];

  while(slot.sequence.load(std::memory_order_acquire) != pos) {
    SpinHint();
  }

  slot.data = event;
  slot.sequence.store(pos + 1, std::memory_order_release);
}

template <typename T, size_t Capacity>
bool MpscRithmicQueue<T, Capacity>::TryPop(T& out) noexcept {
  size_t pos = read_pos_.load(std::memory_order_relaxed);
  Slot& slot = slots_[pos & kMask];

  if(slot.sequence.load(std::memory_order_acquire) != pos + 1) {
    return false;
  }

  out = slot.data;
  read_pos_.store(pos + 1, std::memory_order_release);
  slot.sequence.store(pos + Capacity, std::memory_order_release);
  return true;
}

template <typename T, size_t Capacity>
T MpscRithmicQueue<T, Capacity>::PopBlocking() noexcept {
  while(true) {
    T out;
    if(TryPop(out)) {
      return out;
    }
    SpinHint();
  }
}

}  // namespace rithmic
}  // namespace sqc
