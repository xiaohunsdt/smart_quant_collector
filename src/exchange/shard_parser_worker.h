#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "simdjson.h"
#include "shard_queue.h"
#include "src/common/pmr_pool.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class ShardParserWorker {
 public:
  using TickHandler = std::function<void(std::shared_ptr<TickData>)>;
  using DepthHandler = std::function<void(uint32_t channel_id, const DepthUpdateEvent&)>;

  ShardParserWorker(uint32_t core_id, ShardQueue& input_queue,
                    TickHandler tick_handler = {},
                    DepthHandler depth_handler = {});

  ShardParserWorker(const ShardParserWorker&) = delete;
  ShardParserWorker& operator=(const ShardParserWorker&) = delete;

  void Run();

 private:
  void ParseAndDispatch(const RawMessage& msg);

  uint32_t core_id_;
  ShardQueue& input_queue_;
  PmrPoolManager pool_;
  simdjson::ondemand::parser parser_;
  TickHandler tick_handler_;
  DepthHandler depth_handler_;
};

}  // namespace sqc
