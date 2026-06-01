#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <zmq.hpp>

#include "pub_message.h"
#include "zmq_buffer_pool.h"
#include "src/common/spsc_queue.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

// ── Per-shard queues ──────────────────────────────────────────────
struct ShardQueues {
  SPSCQueue<TickData, 4096> tick;
  SPSCQueue<DepthUpdateEvent, 2048> depth;
  SPSCQueue<BookTickerEvent, 4096> book_ticker;
};

// ── PubWorker (Facade) ─────────────────────────────────────────────
//
// Facade pattern: exposes a simple PublishTick / PublishDepth /
// PublishBookTicker API to parser threads while internally managing
// per-shard SPSC queues, a zero-allocation ZMQ buffer pool, and a
// fair round-robin dispatch loop on the dedicated pub thread.
//
// Dependencies injected via constructor (DI):
//   - zmq::context_t& — shared ZMQ context for PUB socket
//   - topic_prefixes   — channel_id → topic prefix ("exchange:type:symbol")
//   - num_shards       — number of parser threads

class PubWorker {
 public:
  PubWorker(zmq::context_t& ctx,
            std::vector<std::string> topic_prefixes,
            size_t num_shards,
            std::string tcp_endpoint,
            std::string ipc_endpoint);

  PubWorker(const PubWorker&) = delete;
  PubWorker& operator=(const PubWorker&) = delete;

  // ── Publish API (parser threads) ───────────────────────────────
  // Lock-free push into per-shard SPSC queue.  Bounds-checks shard_id;
  // increments dropped_count_ when the queue is full.

  void PublishTick(const TickData& tick, size_t shard);
  void PublishDepth(const DepthUpdateEvent& depth, size_t shard);
  void PublishBookTicker(const BookTickerEvent& bt, size_t shard);

  // ── Lifecycle (pub thread) ─────────────────────────────────────
  void Run();   // connect → dispatch loop → drain → return
  void Stop();  // signal dispatch loop to exit

  // ── Telemetry ──────────────────────────────────────────────────
  uint64_t dropped_count() const {
    return dropped_queue_count_.load(std::memory_order_relaxed) +
           dropped_buffer_count_.load(std::memory_order_relaxed) +
           dropped_hwm_count_.load(std::memory_order_relaxed);
  }
  uint64_t dropped_hwm_count() const {
    return dropped_hwm_count_.load(std::memory_order_relaxed);
  }
  uint64_t dropped_buffer_count() const {
    return dropped_buffer_count_.load(std::memory_order_relaxed);
  }

 private:
  // Serialize + buffer-pool acquire + ZMQ multi-part send.
  template <typename Serializer>
  bool SendTyped(uint32_t channel_id, std::string_view event_suffix,
                 const typename Serializer::value_type& data);

  // Pop and send exactly one item from the queue at position idx.
  bool TrySendOne(size_t shard, size_t queue_type);

  // One round-robin cycle: 1 item per queue, cursor rotates.
  void DispatchCycle(size_t& cursor, bool& any_work);

  // Drain all remaining items after Stop().
  void DrainAll();

  // ── Members ────────────────────────────────────────────────────
  zmq::socket_t pub_socket_;
  std::string tcp_endpoint_;
  std::string ipc_endpoint_;
  ZmqBufferPool buffer_pool_;
  std::unique_ptr<ShardQueues[]> shard_queues_;
  std::vector<std::string> topic_prefixes_;
  size_t num_shards_;

  char topic_buf_[kMaxTopicLen];  // pre-allocated, zero heap in hot path

  std::atomic<uint64_t> dropped_queue_count_{0};
  std::atomic<uint64_t> dropped_buffer_count_{0};
  std::atomic<uint64_t> dropped_hwm_count_{0};
  std::atomic<bool> running_{false};
};

}  // namespace sqc
