#include "gateio_perpetual.h"

#include <cassert>
#include <cstring>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace gateio_perpetual {

EventType PeekEventType(simdjson::ondemand::document& doc) {
  auto ch = doc["channel"].get_string();
  if(ch.error()) return EventType::UNKNOWN;
  std::string_view channel = ch.value_unsafe();
  if(channel == "futures.trades") return EventType::TICK;
  if(channel == "futures.order_book") return EventType::DEPTH;
  if(channel == "futures.book_ticker") return EventType::BOOK_TICKER;
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
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual parse: {}", e.what());
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
    auto arr = doc["result"].get_array();
    auto it = arr.begin();
    if(it == arr.end()) return false;
    auto item = *it;
    out.trade_id = item["id"].get_uint64();
    (void)item["create_time"].get_uint64();
    out.exchange_timestamp = item["create_time_ms"].get_uint64() * 1000ULL;
    out.price = SvToDouble(item["price"].get_string());
    double size_val;
    std::string_view size_sv;
    auto size_err = item["size"].get_string().get(size_sv);
    if(size_err == simdjson::SUCCESS) {
      size_val = SvToDouble(size_sv);
    } else {
      size_val = static_cast<double>(item["size"].get_int64());
    }
    out.quantity = size_val < 0 ? -size_val : size_val;
    out.is_buyer_maker = (size_val < 0);
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: unknown error");
    return false;
  }
}

// ---- Depth (futures.order_book snapshot) ----

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
    out.last_update_id = result["id"].get_uint64();
    out.channel_id = channel_id;
    if(symbol.size() > sizeof(out.symbol) - 1) [[unlikely]] {
    LOG_WARNING(GetLogger(), "Symbol '{}' truncated ({} chars > max {})", symbol, symbol.size(), sizeof(out.symbol)-1);
}
    std::memcpy(out.symbol, symbol.data(), std::min(symbol.size(), sizeof(out.symbol) - 1));
    out.bid_count = 0;
    for(auto lv : result["bids"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q;
      auto s_str = lv["s"].get_string();
      if(s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto lv : result["asks"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q;
      auto s_str = lv["s"].get_string();
      if(s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: unknown error");
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
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual bookTicker parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual bookTicker parse: unknown error");
    return false;
  }
}

}  // namespace gateio_perpetual
}  // namespace sqc
