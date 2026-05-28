#include "shard_parser_worker.h"

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
    if (msg.size == 0) {  // poison pill
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
  auto err = parser_.iterate(msg.buffer(), msg.size, msg.size + kSimdjsonPadding).get(doc);
  if (err) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "iterate err on {} bytes: {}", msg.size, simdjson::error_message(err));
    return;
  }

  try {
    if (std::string_view(msg.exchange) == "binance") {
      auto result = binance_parser::ParseMessage(doc, msg.channel_id);
      if (result.type == binance_parser::ParsedType::TICK && tick_handler_) {
        result.tick.local_timestamp = msg.recv_timestamp;
        tick_handler_(std::move(result.tick));
      } else if (result.type == binance_parser::ParsedType::DEPTH && depth_handler_) {
        result.depth.local_timestamp = msg.recv_timestamp;
        depth_handler_(msg.channel_id, result.depth);
      }
    } else if (std::string_view(msg.exchange) == "gateio") {
      auto result = gateio_parser::ParseMessage(doc, msg.channel_id);
      if (result.type == gateio_parser::ParsedType::TICK && tick_handler_) {
        result.tick.local_timestamp = msg.recv_timestamp;
        tick_handler_(std::move(result.tick));
      } else if (result.type == gateio_parser::ParsedType::DEPTH && depth_handler_) {
        result.depth.local_timestamp = msg.recv_timestamp;
        depth_handler_(msg.channel_id, result.depth);
      }
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
