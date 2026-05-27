#include "shard_parser_worker.h"

#include <chrono>
#include <string>
#include <cstring>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include <exception>
#include "binance/binance_parser.h"
#include "gateio/gateio_parser.h"
#include "src/common/simdjson_utils.h"

namespace sqc {

ShardParserWorker::ShardParserWorker(uint32_t core_id, ShardQueue& input_queue, TickHandler tick_handler, DepthHandler depth_handler) : core_id_(core_id), input_queue_(input_queue), tick_handler_(std::move(tick_handler)), depth_handler_(std::move(depth_handler)) {}

void ShardParserWorker::Run() {
  LOG_INFO(GetLogger(), "ShardParserWorker started on core {}", core_id_);
  while (true) {
    RawMessage msg = input_queue_.PopBlocking();
    if (!msg.data) {
      LOG_INFO(GetLogger(), "ShardParserWorker core {} received poison pill", core_id_);
      break;
    }
    try {
        ParseAndDispatch(msg);
      } catch (...) {
        if (auto* log = GetLogger()) LOG_ERROR(log, "ShardParserWorker core {} unhandled error in Run", core_id_);
      }
  }
  LOG_INFO(GetLogger(), "ShardParserWorker core {} stopped", core_id_);
}

void ShardParserWorker::ParseAndDispatch(const RawMessage& msg) {
  // Buffer already has SIMDJSON_PADDING bytes appended by OnMessage
  simdjson::ondemand::document doc;
  auto err = parser_.iterate(msg.data.get(), msg.size, msg.size + kSimdjsonPadding).get(doc);
  if (err) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "iterate err on {} bytes: {}", msg.size, simdjson::error_message(err));
    return;
  }

  try {
    // Determine event type
    std::string_view event_type;
    if (msg.exchange == "binance") {
      try { event_type = std::string_view(doc["e"]); } catch (...) { return; }
    } else if (msg.exchange == "gateio") {
      try { event_type = std::string_view(doc["channel"]); } catch (...) { return; }
    }

    if (event_type == "trade" || event_type == "aggTrade" || event_type == "futures.tickers") {
      // Trade event
      auto tick = std::allocate_shared<TickData>(pool_.allocator());
      tick->local_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

      bool ok = false;
      if (msg.exchange == "binance")
        ok = binance_parser::ParseTradeEvent(doc, *tick, msg.channel_id);
      else if (msg.exchange == "gateio")
        ok = gateio_parser::ParseTradeEvent(doc, *tick, msg.channel_id);

      if (ok && tick_handler_) tick_handler_(std::move(tick));

    } else if (event_type == "depthUpdate" || event_type == "futures.order_book") {
      // Depth update event
      DepthUpdateEvent depth_event{};
      depth_event.local_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      bool ok = false;
      if (msg.exchange == "binance")
        ok = binance_parser::ParseDepthEvent(doc, depth_event, msg.channel_id);
      else if (msg.exchange == "gateio")
        ok = gateio_parser::ParseDepthEvent(doc, depth_event, msg.channel_id);

      if (ok && depth_handler_) depth_handler_(msg.channel_id, depth_event);
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch simdjson error on core {}: {}", core_id_, e.what());
  } catch (const std::exception& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch exception on core {}: {}", core_id_, e.what());
  } catch (...) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch unknown error on core {}", core_id_);
  }
}

}  // namespace sqc
