#include "binance_common.h"
#include <cstring>
#include <string>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "common/http_utils.h"

namespace sqc {
namespace binance {

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64() * 1000);
    std::string_view sym = doc["s"].get_string();
    out.trade_id = static_cast<uint64_t>(doc["a"].get_int64());
    out.price = SvToDouble(doc["p"].get_string());
    out.quantity = SvToDouble(doc["q"].get_string());
    out.is_buyer_maker = doc["m"].get_bool();
    out.channel_id = channel_id;
    std::memcpy(out.symbol, sym.data(), std::min(sym.size(), sizeof(out.symbol) - 1));
    out.symbol[std::min(sym.size(), sizeof(out.symbol) - 1)] = '\0';
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance trade parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance trade parse: unknown error");
    return false;
  }
}

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    std::string_view sym = doc["s"].get_string();
    out.best_bid_price = SvToDouble(doc["b"].get_string());
    out.best_bid_qty = SvToDouble(doc["B"].get_string());
    out.best_ask_price = SvToDouble(doc["a"].get_string());
    out.best_ask_qty = SvToDouble(doc["A"].get_string());
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64() * 1000);
    out.channel_id = channel_id;
    std::memcpy(out.symbol, sym.data(), std::min(sym.size(), sizeof(out.symbol) - 1));
    out.symbol[std::min(sym.size(), sizeof(out.symbol) - 1)] = '\0';
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance bookTicker parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance bookTicker parse: unknown error");
    return false;
  }
}

// ---- Snapshot fetch + parse (moved from binance_snapshot_client) ----

static bool ParseDepth(const std::string& json, OrderbookSnapshot& out) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto err = parser.parse(json).get(doc);
  if (err) {
    LOG_ERROR(GetLogger(), "Binance depth parse error: {}", simdjson::error_message(err));
    return false;
  }
  uint64_t last_id = 0;
  if (doc["lastUpdateId"].get_uint64().get(last_id) != simdjson::SUCCESS) {
    LOG_ERROR(GetLogger(), "Binance depth missing lastUpdateId");
    return false;
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

OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol, uint32_t limit) {
  OrderbookSnapshot snapshot{};
  bool is_futures = (rest_host.find("fapi") != std::string_view::npos);
  const char* api_path = is_futures ? "/fapi/v1/depth" : "/api/v3/depth";
  std::string target = api_path;
  target += "?symbol=";
  target += symbol;
  target += "&limit=";
  target += std::to_string(limit);
  LOG_INFO(GetLogger(), "BinanceSnapshot: fetching depth {} limit={}", rest_host, limit);
  std::string body = HttpsGet(rest_host, target);
  if (body.empty()) {
    LOG_ERROR(GetLogger(), "BinanceSnapshot: HTTP empty response");
    return snapshot;
  }
  if (!ParseDepth(body, snapshot)) {
    LOG_ERROR(GetLogger(), "BinanceSnapshot: depth parse failed");
  }
  return snapshot;
}

}  // namespace binance
}  // namespace sqc
