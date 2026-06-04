#include "src/exchange/rithmic/rithmic_forwarder.h"

namespace sqc {
namespace rithmic {

RithmicForwarder::RithmicForwarder(uint32_t core_id, MpscRithmicQueue<>& queue,
                                     TickHandler tick_handler, DepthHandler depth_handler,
                                     BookTickerHandler book_ticker_handler)
    : core_id_(core_id),
      queue_(queue),
      tick_handler_(std::move(tick_handler)),
      depth_handler_(std::move(depth_handler)),
      book_ticker_handler_(std::move(book_ticker_handler)) {}

void RithmicForwarder::Run() {
  if (!running_.load(std::memory_order_acquire)) return;
  running_.store(true, std::memory_order_release);

  while (running_.load(std::memory_order_acquire)) {
    RithmicEvent event = queue_.PopBlocking();
    if (!running_.load(std::memory_order_acquire)) break;
    if (event.type == EventType::NONE && event.channel_id == UINT32_MAX) break;

    switch (event.type) {
      case EventType::TICK:   tick_handler_(event.tick); break;
      case EventType::DEPTH:  depth_handler_(event.channel_id, event.depth); break;
      case EventType::BOOK_TICKER: book_ticker_handler_(event.channel_id, event.book_ticker); break;
      default: break;
    }
  }
}

// Fix #9: non-blocking poison pill to avoid deadlock when queue is full
void RithmicForwarder::Stop() {
  running_.store(false, std::memory_order_release);
  RithmicEvent poison;
  poison.type = EventType::NONE;
  poison.channel_id = UINT32_MAX;
  for (int retry = 0; retry < 10; ++retry) {
    if (queue_.TryPush(poison)) return;
  }
}

}  // namespace rithmic
}  // namespace sqc
