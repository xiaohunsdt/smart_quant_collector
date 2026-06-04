#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

#include "src/common/tick_data.h"
#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicForwarder — single consumer of MpscRithmicQueue.
//
// Runs on a dedicated thread. Pops events from the queue and dispatches to
// the same handler lambdas used by ShardParserWorker (no simdjson parsing).
// ============================================================================

class RithmicForwarder {
 public:
  using TickHandler = std::function<void(TickData)>;
  using DepthHandler = std::function<void(uint32_t channel_id, const DepthUpdateEvent&)>;
  using BookTickerHandler = std::function<void(uint32_t channel_id, BookTickerEvent)>;

  RithmicForwarder(uint32_t core_id, MpscRithmicQueue<>& queue,
                   TickHandler tick_handler, DepthHandler depth_handler,
                   BookTickerHandler book_ticker_handler);
  ~RithmicForwarder() = default;

  RithmicForwarder(const RithmicForwarder&) = delete;
  RithmicForwarder& operator=(const RithmicForwarder&) = delete;

  void Run();
  void Stop();
  [[nodiscard]] uint32_t core_id() const noexcept { return core_id_; }

 private:
  uint32_t core_id_;
  MpscRithmicQueue<>& queue_;
  TickHandler tick_handler_;
  DepthHandler depth_handler_;
  BookTickerHandler book_ticker_handler_;
  std::atomic<bool> running_{false};
};

}  // namespace rithmic
}  // namespace sqc
