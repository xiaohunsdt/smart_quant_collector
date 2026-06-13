#include "src/exchange/rithmic/rithmic_receiver.h"

#include <cstring>

#include "quill/LogMacros.h"
#include "src/common/logger_init.h"
#include "src/exchange/rithmic/rithmic_shm.h"

namespace sqc {
namespace rithmic {

RithmicReceiver::RithmicReceiver(shm_layout::TickQueue* tick_queue, shm_layout::DepthQueue* depth_queue,
                                 shm_layout::BookTickerQueue* book_ticker_queue, uint32_t core_id, TickHandler tick_handler,
                                 DepthHandler depth_handler, BookTickerHandler book_ticker_handler)
    : tick_queue_(tick_queue),
      depth_queue_(depth_queue),
      book_ticker_queue_(book_ticker_queue),
      core_id_(core_id),
      tick_handler_(std::move(tick_handler)),
      depth_handler_(std::move(depth_handler)),
      book_ticker_handler_(std::move(book_ticker_handler)) {}

void RithmicReceiver::Run() {
  running_.store(true, std::memory_order_release);

  LOG_INFO(GetLogger(), "RithmicReceiver running on core {}", core_id_);

  while(running_.load(std::memory_order_acquire)) {
    // Poll tick queue first — highest frequency
    {
      TickData tick;
      if(tick_queue_->TryPop(tick)) {
        tick_handler_(tick);
        continue;
      }
    }

    // Poll book ticker queue second — medium frequency
    {
      BookTickerEvent bt;
      if(book_ticker_queue_->TryPop(bt)) {
        book_ticker_handler_(bt.channel_id, bt);
        continue;
      }
    }

    // Poll depth queue last — lowest frequency, largest events
    {
      DepthUpdateEvent depth;
      if(depth_queue_->TryPop(depth)) {
        depth_handler_(depth.channel_id, depth);
        continue;
      }
    }

    // All queues empty — spin-wait hint
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ volatile("yield");
#endif
  }
}

void RithmicReceiver::Stop() { running_.store(false, std::memory_order_release); }

}  // namespace rithmic
}  // namespace sqc
