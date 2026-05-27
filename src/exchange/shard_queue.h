#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sqc {

struct RawMessage {
  std::string data;  // full JSON text, no padding needed (added in parser)
  size_t size = 0;
  uint32_t channel_id = 0;
  std::string exchange;
};

class ShardQueue {
 public:
  static constexpr size_t kDefaultCapacity = 65536;
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
  alignas(64) std::atomic<size_t> write_pos_{0};
  alignas(64) std::atomic<size_t> read_pos_{0};
};

}  // namespace sqc
