#include "gateio_parser.h"
#include <cstring>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "src/common/tick_data.h"

namespace sqc {
namespace gateio_parser {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id, ChannelType /*channel_type*/) {
  ParseResult result;
  try {
    std::string_view channel = doc["channel"].get_string();
    if (channel == "futures.trades" || channel == "spot.trades") {
      result.type = ParsedType::TICK;
      if (!ParseTradeEvent(doc, result.tick, channel_id)) {
        result.type = ParsedType::NONE;
      }
    } else if (channel == "futures.order_book" || channel == "spot.order_book") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id)) {
        result.type = ParsedType::NONE;
      }
    } else if (channel == "futures.order_book_update" || channel == "spot.order_book_update") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthUpdateEvent(doc, result.depth, channel_id)) {
        result.type = ParsedType::NONE;
      }
    } else if (channel == "futures.book_ticker" || channel == "spot.book_ticker") {
      result.type = ParsedType::BOOK_TICKER;
      if (!ParseBookTickerEvent(doc, result.book_ticker, channel_id)) {
        result.type = ParsedType::NONE;
      }
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io ParseMessage: {}", e.what());
  }
  return result;
}

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    // Top-level field order: time, time_ms, channel, event, result
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}

    std::string_view ev = doc["event"].get_string();
    if (ev == "subscribe") return false;

    auto arr = doc["result"].get_array();
    auto it = arr.begin();
    if (it == arr.end()) return false;

    // Only process the first trade in the batch
    auto item = *it;

    // Trade object field order: id, create_time, create_time_ms, price, size, contract, is_internal
    out.trade_id = item["id"].get_uint64();
    (void)item["create_time"].get_uint64();
    out.exchange_timestamp = item["create_time_ms"].get_uint64() * 1000ULL;
    out.price = SvToDouble(item["price"].get_string());
    // size is int64 (unlike price which is a string)
    int64_t size = item["size"].get_int64();
    out.quantity = static_cast<double>(size < 0 ? -size : size);
    out.is_buyer_maker = (size < 0);  // negative = sell = maker

    out.channel_id = channel_id;

    std::string_view sym = item["contract"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io trade parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io trade parse: unknown error");
    return false;
  }
}

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    // Top-level field order: time, time_ms, channel, event, result
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}

    std::string_view ev = doc["event"].get_string();
    if (ev != "update" && ev != "all") return false;

    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.first_update_id = result["id"].get_uint64();
    out.last_update_id = out.first_update_id;
    out.channel_id = channel_id;

    std::string_view sym = result["contract"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';

    out.bid_count = 0;
    for (auto lv : result["bids"]) {
      if (out.bid_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q = static_cast<double>(lv["s"].get_int64());
      out.bids[out.bid_count++] = {p, q};
    }

    out.ask_count = 0;
    for (auto lv : result["asks"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q = static_cast<double>(lv["s"].get_int64());
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io depth parse: unknown error");
    return false;
  }
}

bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}

    std::string_view ev = doc["event"].get_string();
    if (ev != "update") return false;

    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    uint64_t update_id = result["lastUpdateId"].get_uint64();
    // Gate.io update IDs are strictly sequential.
    // Map single lastUpdateId to first/last/prev for lockstep FSM compatibility.
    out.first_update_id = update_id;
    out.last_update_id = update_id;
    out.prev_last_update_id = update_id > 0 ? update_id - 1 : 0;
    out.channel_id = channel_id;

    std::string_view sym;
    auto sym_err = result["s"].get_string().get(sym);
    if (sym_err) {
      sym = result["contract"].get_string();
    }
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';

    out.bid_count = 0;
    for (auto lv : result["bids"]) {
      if (out.bid_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if (it == lv.end()) continue;
      double p = SvToDouble((*it).get_string());
      ++it;
      if (it == lv.end()) continue;
      double q = SvToDouble((*it).get_string());
      out.bids[out.bid_count++] = {p, q};
    }

    out.ask_count = 0;
    for (auto lv : result["asks"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if (it == lv.end()) continue;
      double p = SvToDouble((*it).get_string());
      ++it;
      if (it == lv.end()) continue;
      double q = SvToDouble((*it).get_string());
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io depth update parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io depth update parse: unknown error");
    return false;
  }
}

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}

    std::string_view ev = doc["event"].get_string();
    if (ev != "update") return false;

    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.channel_id = channel_id;

    std::string_view sym;
    auto sym_err = result["s"].get_string().get(sym);
    if (sym_err) {
      sym = result["contract"].get_string();
    }
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';

    auto book = result["book"];
    out.best_bid_price = SvToDouble(book["b"].get_string());
    out.best_bid_qty = static_cast<double>(book["bs"].get_int64());
    out.best_ask_price = SvToDouble(book["a"].get_string());
    out.best_ask_qty = static_cast<double>(book["as"].get_int64());
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io bookTicker parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io bookTicker parse: unknown error");
    return false;
  }
}

}  // namespace gateio_parser
}  // namespace sqc
