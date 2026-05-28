#include "binance_snapshot_client.h"

#include <string>

#include "simdjson.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/http_utils.h"
#include "common/string_utils.h"

namespace sqc {
namespace binance {

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
