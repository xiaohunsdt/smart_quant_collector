#include "gateio_perpetual.h"

#include <cstring>
#include <string>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "common/http_utils.h"

namespace sqc {
namespace gateio_perpetual {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id) {
  ParseResult result;
  try {
    std::string_view channel = doc["channel"].get_string();
    if (channel == "futures.trades") {
      result.type = ParsedType::TICK;
      if (!ParseTradeEvent(doc, result.tick, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "futures.order_book") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "futures.order_book_update") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthUpdateEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "futures.book_ticker") {
      result.type = ParsedType::BOOK_TICKER;
      if (!ParseBookTickerEvent(doc, result.book_ticker, channel_id))
        result.type = ParsedType::NONE;
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual ParseMessage: {}", e.what());
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
    auto arr = doc["result"].get_array();
    auto it = arr.begin();
    if (it == arr.end()) return false;
    auto item = *it;
    out.trade_id = item["id"].get_uint64();
    (void)item["create_time"].get_uint64();
    out.exchange_timestamp = item["create_time_ms"].get_uint64() * 1000ULL;
    out.price = SvToDouble(item["price"].get_string());
    double size_val;
    std::string_view size_sv;
    auto size_err = item["size"].get_string().get(size_sv);
    if (size_err == simdjson::SUCCESS) {
      size_val = SvToDouble(size_sv);
    } else {
      size_val = static_cast<double>(item["size"].get_int64());
    }
    out.quantity = size_val < 0 ? -size_val : size_val;
    out.is_buyer_maker = (size_val < 0);
    out.channel_id = channel_id;
    std::string_view sym = item["contract"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: unknown error");
    return false;
  }
}

// ---- Depth (futures.order_book snapshot) ----

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
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
      double q;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for (auto lv : result["asks"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: unknown error");
    return false;
  }
}

// ---- Depth Update (futures.order_book_update, incremental) ----
// Futures v4: U/u for update IDs, b/a for arrays, objects {p: price, s: size}.
// Size field can be string or int.

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
      double p = SvToDouble(lv["p"].get_string());
      double q;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for (auto lv : result["a"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      double p = SvToDouble(lv["p"].get_string());
      double q;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) {
        q = SvToDouble(s_str.value_unsafe());
      } else {
        q = static_cast<double>(lv["s"].get_int64());
      }
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth update parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth update parse: unknown error");
    return false;
  }
}

// ---- Book Ticker ----
// Futures format: top-level b / B / a / A

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
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual bookTicker parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual bookTicker parse: unknown error");
    return false;
  }
}

// ---- REST Snapshot ----

static bool ParseDepth(const std::string& json, OrderbookSnapshot& out) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto err = parser.parse(json).get(doc);
  if (err) {
    LOG_ERROR(GetLogger(), "Gate.io perpetual depth parse error: {}", simdjson::error_message(err));
    return false;
  }
  uint64_t last_id = 0;
  if (doc["id"].get_uint64().get(last_id) != simdjson::SUCCESS) {
    double d = 0;
    if (doc["current"].get_double().get(d) == simdjson::SUCCESS)
      last_id = static_cast<uint64_t>(d);
  }
  out.lastUpdateId = last_id;

  auto parse_levels = [](simdjson::dom::array arr, PriceLevel* levels, uint32_t& count) {
    count = 0;
    for (auto elem : arr) {
      if (count >= kMaxOrderbookLevels) break;
      simdjson::dom::object obj;
      if (elem.get_object().get(obj) != simdjson::SUCCESS) continue;
      std::string_view psv;
      if (obj["p"].get_string().get(psv) != simdjson::SUCCESS) continue;
      double qty = 0;
      auto s_str = obj["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) {
        qty = SvToDouble(s_str.value_unsafe());
      } else {
        uint64_t q = 0;
        if (obj["s"].get_uint64().get(q) == simdjson::SUCCESS) qty = static_cast<double>(q);
      }
      levels[count].price = SvToDouble(psv);
      levels[count].quantity = qty;
      ++count;
    }
  };

  simdjson::dom::array bids;
  if (doc["bids"].get_array().get(bids) == simdjson::SUCCESS)
    parse_levels(bids, out.bids, out.bid_count);

  simdjson::dom::array asks;
  if (doc["asks"].get_array().get(asks) == simdjson::SUCCESS)
    parse_levels(asks, out.asks, out.ask_count);

  return true;
}

OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  OrderbookSnapshot snapshot{};
  std::string target = "/api/v4/futures/usdt/order_book?contract=";
  target += symbol;
  target += "&limit=100";
  target += "&with_id=true";
  LOG_INFO(GetLogger(), "GateioSnapshot: fetching futures order_book {} limit=100", symbol);
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

}  // namespace gateio_perpetual
}  // namespace sqc
