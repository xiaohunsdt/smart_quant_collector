#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>

namespace sqc {

// Lock-free SPSC (Single Producer, Single Consumer) ring buffer.
//
// Template parameters:
//   T          — element type (must be trivially copyable)
//   kCapacity  — queue depth (must be power of 2, default 1024)
//
// Cache-line isolation: write_pos_ and read_pos_ are on separate cache
// lines to eliminate false sharing between producer and consumer threads.
//
// Usage:
//   Producer thread:  while (!q.try_push(item)) { /* backoff */ }
//   Consumer thread:  while (!q.try_pop(item)) { /* backoff */ }
template <typename T, size_t kCapacity = 1024>
class SPSCQueue {
  static_assert((kCapacity & (kCapacity - 1)) == 0,
                "SPSCQueue capacity must be a power of 2");
  static_assert(std::is_trivially_copyable_v<T>,
                "SPSCQueue element type must be trivially copyable");

 public:
  static constexpr size_t capacity = kCapacity;
  static constexpr size_t kMask = kCapacity - 1;

  SPSCQueue() = default;
  SPSCQueue(const SPSCQueue&) = delete;
  SPSCQueue& operator=(const SPSCQueue&) = delete;

  // Non-blocking push. Returns false if the queue is full.
  // Both counters are monotonic (never wrap) — the mask is applied only
  // for array indexing.  This guarantees correct full/empty detection
  // after any number of push/pop cycles.
  [[nodiscard]] bool try_push(const T& item) noexcept {
    const size_t w = write_pos_.load(std::memory_order_relaxed);
    const size_t r = read_pos_.load(std::memory_order_acquire);
    if ((w - r) >= kCapacity) {
      return false;  // full
    }
    slots_[w & kMask] = item;
    write_pos_.store(w + 1, std::memory_order_release);
    return true;
  }

  // Non-blocking pop. Returns false if the queue is empty.
  [[nodiscard]] bool try_pop(T& out) noexcept {
    const size_t r = read_pos_.load(std::memory_order_relaxed);
    const size_t w = write_pos_.load(std::memory_order_acquire);
    if (r == w) {
      return false;  // empty
    }
    out = slots_[r & kMask];
    read_pos_.store(r + 1, std::memory_order_release);
    return true;
  }

 private:
  alignas(64) T slots_[kCapacity];
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
};

}  // namespace sqc
