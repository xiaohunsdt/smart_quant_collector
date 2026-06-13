#include "shard_parser_worker.h"

#include <cstring>
#include <exception>
#include <utility>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"
#include "src/common/simdjson_utils.h"

namespace sqc {

ShardParserWorker::ShardParserWorker(uint32_t core_id, ShardQueue& input_queue, TickHandler tick_handler, DepthHandler depth_handler,
                                     BookTickerHandler book_ticker_handler)
    : core_id_(core_id),
      input_queue_(input_queue),
      tick_handler_(std::move(tick_handler)),
      depth_handler_(std::move(depth_handler)),
      book_ticker_handler_(std::move(book_ticker_handler)) {}

void ShardParserWorker::Run() {
  LOG_INFO(GetLogger(), "ShardParserWorker started on core {}", core_id_);
  while(true) {
    RawMessage msg = input_queue_.PopBlocking();
    if(msg.size == 0) {  // poison pill
      LOG_INFO(GetLogger(), "ShardParserWorker core {} received poison pill", core_id_);
      break;
    }
    try {
      ParseAndDispatch(msg);
    } catch(...) {
      if(auto* log = GetLogger()) LOG_ERROR(log, "ShardParserWorker core {} unhandled error in Run", core_id_);
    }
  }
  LOG_INFO(GetLogger(), "ShardParserWorker core {} stopped", core_id_);
}

void ShardParserWorker::ParseAndDispatch(const RawMessage& msg) {
  simdjson::ondemand::document doc;
  auto err = parser_.iterate(msg.buffer(), msg.size, msg.size + kSimdjsonPadding).get(doc);
  if(err) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "iterate err on {} bytes: {}", msg.size, simdjson::error_message(err));
    return;
  }

  try {
    if(!msg.parse_fn) return;
    auto result = msg.parse_fn(doc, msg.channel_id, msg.event_type);

    // Stamp symbol from RawMessage (pre-fetched on network thread) into all
    // result structs.  parse_fn no longer accepts symbol, so we do it here
    // after the call — one place for all adapters, zero extra indirections.
    const std::string_view sym(msg.symbol, std::strlen(msg.symbol));

    switch(result.type) {
      case ParsedType::TICK:
        if(tick_handler_) {
          CopySymbol(result.tick.symbol, sizeof(result.tick.symbol), sym);
          result.tick.local_diff = msg.recv_timestamp;
          tick_handler_(std::move(result.tick));
        }
        break;
      case ParsedType::DEPTH:
        if(depth_handler_) {
          CopySymbol(result.depth.symbol, sizeof(result.depth.symbol), sym);
          result.depth.local_diff = msg.recv_timestamp;
          depth_handler_(msg.channel_id, result.depth);
        }
        break;
      case ParsedType::BOOK_TICKER:
        if(book_ticker_handler_) {
          CopySymbol(result.book_ticker.symbol, sizeof(result.book_ticker.symbol), sym);
          result.book_ticker.local_diff = msg.recv_timestamp;
          book_ticker_handler_(msg.channel_id, result.book_ticker);
        }
        break;
      case ParsedType::NONE:
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch simdjson error on core {}: {}", core_id_, e.what());
  } catch(const std::exception& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch exception on core {}: {}", core_id_, e.what());
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "ParseAndDispatch unknown error on core {}", core_id_);
  }
}

}  // namespace sqc
