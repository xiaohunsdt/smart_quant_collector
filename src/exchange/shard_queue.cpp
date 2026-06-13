#include "shard_queue.h"

#include <thread>

namespace sqc {

namespace {
size_t NextPowerOf2(size_t n) {
  size_t p = 1;
  while(p < n) p <<= 1;
  return p;
}
}  // namespace

ShardQueue::ShardQueue(size_t capacity) : capacity_(NextPowerOf2(capacity)), mask_(capacity_ - 1), slots_(capacity_) {}

bool ShardQueue::TryPush(RawMessage msg) {
  // Single-producer path: relaxed load of write_pos_ is safe because only
  // one thread (the network thread) ever writes it.  Acquire on read_pos_ to
  // synchronise-with the consumer's release store so we see the latest drain.
  const size_t w = write_pos_.load(std::memory_order_relaxed);
  const size_t r = read_pos_.load(std::memory_order_acquire);
  if(w - r >= capacity_) return false;
  slots_[w & mask_] = std::move(msg);
  write_pos_.store(w + 1, std::memory_order_release);
  return true;
}

void ShardQueue::Push(RawMessage msg) {
  while(!TryPush(std::move(msg))) std::this_thread::yield();
}

bool ShardQueue::TryPop(RawMessage& out) {
  size_t r = read_pos_.load(std::memory_order_relaxed);
  size_t w = write_pos_.load(std::memory_order_acquire);
  if(r >= w) return false;
  out = std::move(slots_[r & mask_]);
  read_pos_.store(r + 1, std::memory_order_release);
  return true;
}

RawMessage ShardQueue::PopBlocking() {
  RawMessage msg;
  while(!TryPop(msg)) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile("yield");
#else
    std::this_thread::yield();
#endif
  }
  return msg;
}

void ShardQueue::PushPoisonPill() { Push(RawMessage{}); }

}  // namespace sqc
