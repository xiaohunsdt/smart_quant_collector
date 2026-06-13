#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

#include "src/common/tick_data.h"
#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicReceiver — parent-side consumer of 3 shared-memory MPSC queues.
//
// Runs on a dedicated thread. Round-robin TryPop from tick, book_ticker, and
// depth queues — no serialization, no type discriminator, no system calls on
// the hot path.
// ============================================================================

class RithmicReceiver {
 public:
  using TickHandler = std::function<void(TickData)>;
  using DepthHandler = std::function<void(uint32_t channel_id, const DepthUpdateEvent&)>;
  using BookTickerHandler = std::function<void(uint32_t channel_id, BookTickerEvent)>;

  RithmicReceiver(shm_layout::TickQueue* tick_queue,
                  shm_layout::DepthQueue* depth_queue,
                  shm_layout::BookTickerQueue* book_ticker_queue,
                  uint32_t core_id,
                  TickHandler tick_handler,
                  DepthHandler depth_handler,
                  BookTickerHandler book_ticker_handler);

  ~RithmicReceiver() = default;

  RithmicReceiver(const RithmicReceiver&) = delete;
  RithmicReceiver& operator=(const RithmicReceiver&) = delete;

  void Run();
  void Stop();

  [[nodiscard]] uint32_t core_id() const noexcept { return core_id_; }

 private:
  shm_layout::TickQueue* tick_queue_;
  shm_layout::DepthQueue* depth_queue_;
  shm_layout::BookTickerQueue* book_ticker_queue_;
  uint32_t core_id_;
  TickHandler tick_handler_;
  DepthHandler depth_handler_;
  BookTickerHandler book_ticker_handler_;
  std::atomic<bool> running_{false};
};

}  // namespace rithmic
}  // namespace sqc
