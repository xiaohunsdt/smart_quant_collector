#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "src/common/simdjson_utils.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {

struct RawMessage {
  static constexpr size_t kInlineSize = 16384;

  char* buffer() { return heap_ ? heap_.get() : inline_; }
  const char* buffer() const { return heap_ ? heap_.get() : inline_; }
  size_t capacity() const { return heap_ ? heap_size_ : kInlineSize; }

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
  uint64_t recv_timestamp = 0;
  char exchange[16] = {};

  ParseResult (*parse_fn)(simdjson::ondemand::document& doc, uint32_t channel_id) = nullptr;
};

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

 private:
  size_t capacity_;
  size_t mask_;
  std::vector<RawMessage> slots_;
  std::mutex push_mtx_;
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
};

}  // namespace sqc
