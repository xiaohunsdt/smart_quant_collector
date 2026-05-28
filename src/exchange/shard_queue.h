#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "src/common/simdjson_utils.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {

// Inline-buffer message: eliminates per-message heap allocation on the hot
// path (F09).  16 KiB covers ~99 % of exchange WebSocket frames; oversized
// messages fall back to a heap buffer.
struct RawMessage {
  static constexpr size_t kInlineSize = 16384;

  // Accessor: returns the active data pointer (inline or heap).
  char* buffer() { return heap_ ? heap_.get() : inline_; }
  const char* buffer() const { return heap_ ? heap_.get() : inline_; }

  // Capacity of the active buffer (including SIMDJSON_PADDING).
  size_t capacity() const { return heap_ ? heap_size_ : kInlineSize; }

  // Prepare buffer for `needed` bytes of payload + SIMDJSON_PADDING.
  // Returns false if allocation fails.
  bool allocate(size_t needed) noexcept {
    const size_t total = needed + kSimdjsonPadding;
    if (total <= kInlineSize) {
      heap_.reset();
      heap_size_ = 0;
      return true;
    }
    try {
      heap_ = std::make_unique<char[]>(total);
    } catch (const std::bad_alloc&) {
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
  uint64_t recv_timestamp = 0;  // steady_clock ns, set at OnMessage
  char exchange[16] = {};       // fixed-size, zero-allocation
  ChannelType channel_type = ChannelType::Spot;

  // Hot-path parse function pointer (set by SymbolChannel::OnMessage).
  ParseResult (*parse_fn)(simdjson::ondemand::document& doc, uint32_t channel_id, ChannelType channel_type) = nullptr;
};

// Ring-buffer queue with MPSC safety on the producer side.
// Architecture: multiple SymbolChannels (network thread) + shutdown thread
// may push; exactly one parser worker pops per queue.
class ShardQueue {
 public:
  // Reduced from 65536 → 4096 because each RawMessage slot is now ~16 KiB.
  static constexpr size_t kDefaultCapacity = 4096;
  explicit ShardQueue(size_t capacity = kDefaultCapacity);

  ShardQueue(const ShardQueue&) = delete;
  ShardQueue& operator=(const ShardQueue&) = delete;

  bool TryPush(RawMessage msg);
  void Push(RawMessage msg);
  bool TryPop(RawMessage& out);
  RawMessage PopBlocking();
  void PushPoisonPill();

 private:
  size_t capacity_;
  size_t mask_;
  std::vector<RawMessage> slots_;

  // Producer-side lock: multiple producers (network thread + shutdown) may
  // contend, but contention is low (batch pushes, poison pill rare).
  std::mutex push_mtx_;
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
};

}  // namespace sqc
