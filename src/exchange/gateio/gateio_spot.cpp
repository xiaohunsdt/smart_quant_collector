#include "gateio_spot.h"

#include <cstring>
#include <string>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "common/http_utils.h"

namespace sqc {
namespace gateio_spot {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id) {
  ParseResult result;
  try {
    std::string_view channel = doc["channel"].get_string();
    if (channel == "spot.trades") {
      result.type = ParsedType::TICK;
      if (!ParseTradeEvent(doc, result.tick, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.order_book") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.order_book_update") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthUpdateEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.book_ticker") {
      result.type = ParsedType::BOOK_TICKER;
      if (!ParseBookTickerEvent(doc, result.book_ticker, channel_id))
        result.type = ParsedType::NONE;
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot ParseMessage: {}", e.what());
  }
  return result;
}

// ---- Trade ----

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}
    std::string_view ev = doc["event"].get_string();
    if (ev == "subscribe") return false;
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
    std::string_view sym = result["currency_pair"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: unknown error");
    return false;
  }
}

// ---- Depth (spot.order_book snapshot) ----

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}
    std::string_view ev = doc["event"].get_string();
    if (ev != "update" && ev != "all") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.first_update_id = result["lastUpdateId"].get_uint64();
    out.last_update_id = out.first_update_id;
    out.channel_id = channel_id;
    std::string_view sym = result["s"].get_string();
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
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: unknown error");
    return false;
  }
}

// ---- Depth Update (spot.order_book_update, incremental) ----
// Spot v4 format: U/u for update IDs, b/a for arrays, flat [price, qty] entries.

bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}
    std::string_view ev = doc["event"].get_string();
    if (ev != "update") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.first_update_id = result["U"].get_uint64();
    out.last_update_id = result["u"].get_uint64();
    out.prev_last_update_id = out.first_update_id > 0 ? out.first_update_id - 1 : 0;
    out.channel_id = channel_id;
    std::string_view sym = result["s"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
    out.bid_count = 0;
    for (auto lv : result["b"]) {
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
    for (auto lv : result["a"]) {
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
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth update parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth update parse: unknown error");
    return false;
  }
}

// ---- Book Ticker ----
// Spot format: nested book.b / book.bs / book.a / book.as

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try { (void)doc["time_ms"].get_uint64(); } catch (...) {}
    std::string_view ev = doc["event"].get_string();
    if (ev != "update") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.channel_id = channel_id;
    std::string_view sym = result["s"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
    out.best_bid_price = SvToDouble(result["b"].get_string());
    auto bid_sz = result["B"].get_string();
    out.best_bid_qty = bid_sz.error() == simdjson::SUCCESS
        ? SvToDouble(bid_sz.value_unsafe())
        : static_cast<double>(result["B"].get_int64());
    out.best_ask_price = SvToDouble(result["a"].get_string());
    auto ask_sz = result["A"].get_string();
    out.best_ask_qty = ask_sz.error() == simdjson::SUCCESS
        ? SvToDouble(ask_sz.value_unsafe())
        : static_cast<double>(result["A"].get_int64());
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot bookTicker parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot bookTicker parse: unknown error");
    return false;
  }
}

// ---- REST Snapshot ----

static bool ParseDepth(const std::string& json, OrderbookSnapshot& out) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto err = parser.parse(json).get(doc);
  if (err) {
    LOG_ERROR(GetLogger(), "Gate.io spot depth parse error: {}", simdjson::error_message(err));
    return false;
  }
  uint64_t last_id = 0;
  if (doc["id"].get_uint64().get(last_id) != simdjson::SUCCESS) {
    (void)doc["current"].get_uint64().get(last_id);
  }
  out.lastUpdateId = last_id;
  simdjson::dom::array bids;
  if (doc["bids"].get_array().get(bids) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : bids) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.bids[i].price = SvToDouble(psv);
      out.bids[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.bid_count = i;
  }
  simdjson::dom::array asks;
  if (doc["asks"].get_array().get(asks) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : asks) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.asks[i].price = SvToDouble(psv);
      out.asks[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.ask_count = i;
  }
  return true;
}

OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  OrderbookSnapshot snapshot{};
  std::string target = "/api/v4/spot/order_book?currency_pair=";
  target += symbol;
  target += "&limit=50";
  target += "&with_id=true";
  LOG_INFO(GetLogger(), "GateioSnapshot: fetching spot order_book {} limit=50", symbol);
  std::string body = HttpsGet(rest_host, target);
  if (body.empty()) {
    LOG_ERROR(GetLogger(), "GateioSnapshot: HTTP empty response");
    return snapshot;
  }
  if (!ParseDepth(body, snapshot)) {
    LOG_ERROR(GetLogger(), "GateioSnapshot: depth parse failed");
  }
  return snapshot;
}

}  // namespace gateio_spot
}  // namespace sqc
