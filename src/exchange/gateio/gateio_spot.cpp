#include "gateio_spot.h"

#include <cassert>
#include <cstring>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace gateio_spot {

EventType PeekEventType(simdjson::ondemand::document& doc) {
  auto ch = doc["channel"].get_string();
  if(ch.error()) return EventType::UNKNOWN;
  std::string_view channel = ch.value_unsafe();
  if(channel == "spot.trades") return EventType::TICK;
  if(channel == "spot.order_book") return EventType::DEPTH;
  if(channel == "spot.book_ticker") return EventType::BOOK_TICKER;
  return EventType::UNKNOWN;
}

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view symbol, EventType event_type) {
  ParseResult result;
  try {
    switch(event_type) {
      case EventType::TICK:
        result.type = ParsedType::TICK;
        if(!ParseTradeEvent(doc, result.tick, channel_id, symbol)) result.type = ParsedType::NONE;
        break;
      case EventType::DEPTH:
        result.type = ParsedType::DEPTH;
        if(!ParseDepthEvent(doc, result.depth, channel_id, symbol)) result.type = ParsedType::NONE;
        break;
      case EventType::BOOK_TICKER:
        result.type = ParsedType::BOOK_TICKER;
        if(!ParseBookTickerEvent(doc, result.book_ticker, channel_id, symbol)) result.type = ParsedType::NONE;
        break;
      default:
        result.type = ParsedType::NONE;
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot parse: {}", e.what());
  }
  return result;
}

// ---- Trade ----

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id, std::string_view symbol) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev == "subscribe") return false;
    auto result = doc["result"];
    out.trade_id = result["id"].get_uint64();
    (void)result["create_time"].get_uint64();
    out.exchange_timestamp = static_cast<uint64_t>(SvToDouble(result["create_time_ms"].get_string()) * 1000.0);
    out.price = SvToDouble(result["price"].get_string());
    std::string_view side = result["side"].get_string();
    double qty = SvToDouble(result["amount"].get_string());
    out.quantity = qty < 0 ? -qty : qty;
    out.is_buyer_maker = (side == "sell");
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: unknown error");
    return false;
  }
}

// ---- Depth (spot.order_book snapshot) ----

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id, std::string_view symbol) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev != "update" && ev != "all") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.last_update_id = result["lastUpdateId"].get_uint64();
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));
    out.bid_count = 0;
    for(auto lv : result["bids"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if(it == lv.end()) continue;
      double p = SvToDouble((*it).get_string());
      ++it;
      if(it == lv.end()) continue;
      double q = SvToDouble((*it).get_string());
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto lv : result["asks"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if(it == lv.end()) continue;
      double p = SvToDouble((*it).get_string());
      ++it;
      if(it == lv.end()) continue;
      double q = SvToDouble((*it).get_string());
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: unknown error");
    return false;
  }
}

// ---- Book Ticker ----

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id, std::string_view symbol) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev != "update") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.best_bid_price = SvToDouble(result["b"].get_string());
    auto bid_sz = result["B"].get_string();
    out.best_bid_qty = bid_sz.error() == simdjson::SUCCESS ? SvToDouble(bid_sz.value_unsafe()) : static_cast<double>(result["B"].get_int64());
    out.best_ask_price = SvToDouble(result["a"].get_string());
    auto ask_sz = result["A"].get_string();
    out.best_ask_qty = ask_sz.error() == simdjson::SUCCESS ? SvToDouble(ask_sz.value_unsafe()) : static_cast<double>(result["A"].get_int64());
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot bookTicker parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot bookTicker parse: unknown error");
    return false;
  }
}

}  // namespace gateio_spot
}  // namespace sqc
