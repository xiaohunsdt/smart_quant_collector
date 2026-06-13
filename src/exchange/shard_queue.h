#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "src/common/simdjson_utils.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {

struct RawMessage {
  static constexpr size_t kInlineSize = 16384;

  char* buffer() { return heap_ ? heap_.get() : inline_; }
  const char* buffer() const { return heap_ ? heap_.get() : inline_; }
  size_t capacity() const { return heap_ ? heap_size_ : kInlineSize; }

  [[nodiscard]] bool allocate(size_t needed) noexcept {
    const size_t total = needed + kSimdjsonPadding;
    if(total <= kInlineSize) {
      heap_.reset();
      heap_size_ = 0;
      return true;
    }
    try {
      heap_ = std::make_unique<char[]>(total);
    } catch(const std::bad_alloc&) {
      return false;
    }
    heap_size_ = total;
    return true;
  }

  alignas(64) char inline_[kInlineSize];
  std::unique_ptr<char[]> heap_;
  size_t heap_size_ = 0;
  size_t size = 0;
  uint32_t channel_id = 0;
  uint64_t recv_timestamp = 0;
  char symbol[32] = {};  // accommodates long symbols with suffix (e.g. BTCUSDT_PERP)
  EventType event_type = EventType::TICK;

  ParseResult (*parse_fn)(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type) = nullptr;
};

// Single-producer / single-consumer lock-free ring buffer for RawMessage.
//
// CONSTRAINT: Push / TryPush MUST be called from a single thread.  All
// callers originate from the network thread (Boost.Asio io_context::run()),
// which serialises callbacks, so this invariant always holds.
// If multi-producer support is ever needed, upgrade to the Vyukov MPSC queue
// already in src/exchange/rithmic/rithmic_queue.h.
class ShardQueue {
 public:
  static constexpr size_t kDefaultCapacity = 4096;
  explicit ShardQueue(size_t capacity = kDefaultCapacity);

  ShardQueue(const ShardQueue&) = delete;
  ShardQueue& operator=(const ShardQueue&) = delete;

  bool TryPush(RawMessage msg);
  void Push(RawMessage msg);
  bool TryPop(RawMessage& out);
  RawMessage PopBlocking();
  void PushPoisonPill();

  /// Approximate queue depth (consumer thread). Returns write_pos - read_pos.
  /// The read is racy — suitable for telemetry gauges, not for exact accounting.
  [[nodiscard]] size_t size() const noexcept {
    size_t w = write_pos_.load(std::memory_order_acquire);
    size_t r = read_pos_.load(std::memory_order_relaxed);
    return w - r;
  }

 private:
  alignas(64) std::atomic<size_t> write_pos_{0};  // hot — producer (own cache line)
  size_t capacity_;
  size_t mask_;
  std::vector<RawMessage> slots_;
  alignas(64) std::atomic<size_t> read_pos_{0};    // hot — consumer (own cache line)
};

}  // namespace sqc
