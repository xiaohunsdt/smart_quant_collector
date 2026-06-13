#pragma once

#include <atomic>
#include <cstdint>

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
// Non-blocking TryPush() drops events when the queue is full (increments
// atomic dropped_count_).
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
  /// Returns false if the queue is full (event is DROPPED).
  /// Non-blocking — safe to call from REngine callback threads.
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
  size_t pos = write_pos_.fetch_add(1, std::memory_order_relaxed);
  Slot& slot = slots_[pos & kMask];

  uint64_t seq = slot.sequence.load(std::memory_order_acquire);
  if(seq != pos) {
    int spin = 0;
    while(slot.sequence.load(std::memory_order_acquire) != pos) {
      if(++spin > 10000) {
        write_pos_.fetch_sub(1, std::memory_order_relaxed);
        dropped_count_.fetch_add(1, std::memory_order_relaxed);
        return false;
      }
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
      __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
      __asm__ volatile("yield");
#endif
    }
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
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile("yield");
#endif
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
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile("yield");
#endif
  }
}

}  // namespace rithmic
}  // namespace sqc
