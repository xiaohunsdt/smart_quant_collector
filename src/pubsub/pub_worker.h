#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include <zmq.hpp>

#include "src/common/tick_data.h"

namespace sqc {

// SPSC ring buffer for TickData.
// Single producer (parser worker), single consumer (pub thread).
// Zero-allocation, lock-free, cache-line padded — same pattern as ShardQueue.
struct TickSPSCQueue {
  static constexpr size_t kCapacity = 256;
  static constexpr size_t kMask = kCapacity - 1;
  static_assert((kCapacity & kMask) == 0, "capacity must be power of 2");

  bool try_push(const TickData& tick) {
    size_t w = write_pos_.load(std::memory_order_relaxed);
    size_t next = (w + 1) & kMask;
    if (next == read_pos_.load(std::memory_order_acquire)) return false;  // full
    slots_[w] = tick;
    write_pos_.store(next, std::memory_order_release);
    return true;
  }

  bool try_pop(TickData& out) {
    size_t r = read_pos_.load(std::memory_order_relaxed);
    if (r == write_pos_.load(std::memory_order_acquire)) return false;  // empty
    out = slots_[r];
    read_pos_.store((r + 1) & kMask, std::memory_order_release);
    return true;
  }

  TickData slots_[kCapacity];
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
};

// ZeroMQ publisher worker.
// Multiple parser threads push ticks into per-parser SPSC queues (lock-free).
// Dedicated pub thread (single-threaded) reads from all queues and sends via ZMQ.
class PubWorker {
 public:
  explicit PubWorker(const std::string& endpoint);

  PubWorker(const PubWorker&) = delete;
  PubWorker& operator=(const PubWorker&) = delete;

  // Pre-allocate SPSC queues (one per parser worker).
  void Init(size_t num_queues);

  // Lock-free push from parser thread (shard_id = parser index).
  void PublishTick(const TickData& tick, size_t shard_id);

  // Run the publish loop on the dedicated pub thread. Returns on Stop().
  void Run();

  // Signal Run() to exit after draining queues.
  void Stop();

  uint64_t dropped_count() const { return dropped_count_.load(std::memory_order_relaxed); }

 private:
  zmq::context_t ctx_;
  zmq::socket_t pub_socket_;
  std::atomic<uint64_t> dropped_count_{0};
  std::atomic<bool> running_{false};
  std::unique_ptr<TickSPSCQueue[]> queues_;
  size_t num_queues_ = 0;
};

}  // namespace sqc
