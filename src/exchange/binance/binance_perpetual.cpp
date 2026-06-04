#include "binance_perpetual.h"

#include <cassert>
#include <cstring>

#include "binance_common.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace binance_perpetual {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view symbol, EventType event_type) {
  ParseResult result;
  try {
    switch(event_type) {
      case EventType::TICK:
        result.type = ParsedType::TICK;
        if(!binance::ParseTradeEvent(doc, result.tick, channel_id, symbol))
          result.type = ParsedType::NONE;
        else
          result.tick.exchange_timestamp *= 1000ULL;  // perpetual: ms → μs
        break;
      case EventType::DEPTH:
        // Futures partial depth uses same "depthUpdate" format
        result.type = ParsedType::DEPTH;
        if(!ParseDepthEvent(doc, result.depth, channel_id, symbol)) result.type = ParsedType::NONE;
        break;
      case EventType::BOOK_TICKER:
        result.type = ParsedType::BOOK_TICKER;
        if(!binance::ParseBookTickerEvent(doc, result.book_ticker, channel_id, symbol)) result.type = ParsedType::NONE;
        break;
      default:
        result.type = ParsedType::NONE;
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual parse: {}", e.what());
  }
  return result;
}

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id, std::string_view symbol) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64()) * 1000ULL;
    out.last_update_id = static_cast<uint64_t>(doc["u"].get_int64());
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));

    out.bid_count = 0;
    for(auto bid_level : doc["b"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = bid_level.begin();
      if(it != bid_level.end()) {
        p = SvToDouble((*it).get_string());
      }
      ++it;
      if(it != bid_level.end()) {
        q = SvToDouble((*it).get_string());
      }
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto ask_level : doc["a"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = ask_level.begin();
      if(it != ask_level.end()) {
        p = SvToDouble((*it).get_string());
      }
      ++it;
      if(it != ask_level.end()) {
        q = SvToDouble((*it).get_string());
      }
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual depth parse: unknown error");
    return false;
  }
}

}  // namespace binance_perpetual
}  // namespace sqc
