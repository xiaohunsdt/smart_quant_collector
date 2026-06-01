#include "zmq_buffer_pool.h"

namespace sqc {

void* ZmqBufferPool::Acquire() noexcept {
  for (size_t attempt = 0; attempt < kPoolSize; ++attempt) {
    const size_t idx =
        next_.fetch_add(1, std::memory_order_relaxed) % kPoolSize;
    bool expected = false;
    if (pool_[idx].in_use.compare_exchange_strong(
            expected, true, std::memory_order_acquire,
            std::memory_order_relaxed)) {
      return pool_[idx].data;
    }
  }
  return nullptr;  // pool exhausted — natural backpressure
}

void ZmqBufferPool::Release(void* data, void* hint) noexcept {
  (void)data;  // zmq_msg_init_data passes the buffer pointer; we use hint instead
  auto* slot = static_cast<Slot*>(hint);
  slot->in_use.store(false, std::memory_order_release);
}

}  // namespace sqc
