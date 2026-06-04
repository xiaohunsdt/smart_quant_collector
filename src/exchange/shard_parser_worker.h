#pragma once

#include <cstdint>
#include <functional>

#include "shard_queue.h"
#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class ShardParserWorker {
 public:
  using TickHandler = std::function<void(TickData)>;
  using DepthHandler = std::function<void(uint32_t channel_id, const DepthUpdateEvent&)>;
  using BookTickerHandler = std::function<void(uint32_t channel_id, BookTickerEvent)>;

  ShardParserWorker(uint32_t core_id, ShardQueue& input_queue, TickHandler tick_handler = {}, DepthHandler depth_handler = {},
                    BookTickerHandler book_ticker_handler = {});

  ShardParserWorker(const ShardParserWorker&) = delete;
  ShardParserWorker& operator=(const ShardParserWorker&) = delete;

  void Run();

 private:
  void ParseAndDispatch(const RawMessage& msg);

  uint32_t core_id_;
  ShardQueue& input_queue_;
  simdjson::ondemand::parser parser_;
  TickHandler tick_handler_;
  DepthHandler depth_handler_;
  BookTickerHandler book_ticker_handler_;
};

}  // namespace sqc
