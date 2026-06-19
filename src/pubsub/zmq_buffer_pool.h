#pragma once

#include <atomic>
#include <cstddef>

#include "pub_message.h"

namespace sqc {

// ── Zero-allocation ZMQ buffer pool ────────────────────────────────
//
// Object Pool pattern: pre-allocates a fixed number of buffers sized
// for the largest possible ZMQ payload frame.  Buffers are acquired
// via lock-free CAS and returned via a callback passed to
// zmq_msg_init_data(), eliminating all heap allocation from the ZMQ
// send hot path.
//
// Pool exhaustion acts as natural backpressure: when all buffers are
// in flight (waiting for ZMQ to complete transmission), Acquire()
// returns nullptr and the caller must drop the message.
//
// Thread safety: Acquire() is safe to call from a single consumer
// thread (the pub thread).  Release() is called from ZMQ's internal
// I/O thread via the free_fn callback and uses atomic store to mark
// the slot available.

class ZmqBufferPool {
 public:
  static constexpr size_t kPoolSize = 64;
  static constexpr size_t kBufferSize = kMaxPayloadSize;

  // A pooled buffer slot. Exposed (not nested-private) because callers acquire
  // a Slot* and serialize directly into slot->data before handing the Slot* to
  // ZMQ as the zero-copy hint — Release() uses it to clear in_use.
  struct Slot {
    alignas(64) char data[kBufferSize];
    std::atomic<bool> in_use{false};
  };

  ZmqBufferPool() = default;
  ZmqBufferPool(const ZmqBufferPool&) = delete;
  ZmqBufferPool& operator=(const ZmqBufferPool&) = delete;

  // Acquire a buffer from the pool.  Returns nullptr if the pool is
  // exhausted (all buffers are in flight).  The returned Slot* owns a
  // `data` buffer valid until Release() is called on the Slot.
  [[nodiscard]] Slot* Acquire() noexcept;

  // Callback for zmq_msg_init_data().  ZMQ calls this when it is done
  // with the buffer (after the message has been sent or discarded).
  // `hint` is the Slot* passed at acquire time — never reinterpret the
  // data pointer itself (its address only coincides with Slot when
  // `data` is the first member).
  static void Release(void* data, void* hint) noexcept;

 private:
  Slot pool_[kPoolSize];
  std::atomic<size_t> next_{0};
};

}  // namespace sqc
