#include "gateio_parser.h"
#include <cstring>
#include <string>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/common/tick_data.h"

namespace sqc {
namespace gateio_parser {

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    std::string_view ev = std::string_view(doc["event"]);
    if (ev == "subscribe") return false;
    auto arr = doc["result"]; auto it = arr.begin();
    if (it == arr.end()) return false;
    auto item = *it;

    out.exchange_timestamp = static_cast<uint64_t>(doc["time"].get_double() * 1'000'000.0);
    out.price = item["last"].get_double();
    out.quantity = item["volume_24h"].get_double();
    out.trade_id = item["total_size"].get_uint64();
    out.channel_id = channel_id;
    std::string_view sym = item["contract"].get_string();
    std::strncpy(out.symbol, sym.data(), sizeof(out.symbol) - 1);
    out.symbol[sizeof(out.symbol) - 1] = '\0';
    out.is_buyer_maker = false;
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
    std::string_view ev = std::string_view(doc["event"]);
    if (ev != "update") return false;

    auto result = doc["result"];
    out.U = result["id"].get_uint64(); out.u = out.U;
    out.channel_id = channel_id;
    out.exchange_timestamp = static_cast<uint64_t>(doc["time"].get_double() * 1'000'000.0);
    std::string_view sym = result["contract"].get_string();
    std::strncpy(out.symbol, sym.data(), sizeof(out.symbol) - 1);

    out.bid_count = 0;
    for (auto lv : result["bids"]) {
      if (out.bid_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if (it == lv.end()) continue;
      double p = (*it).get_double();
      ++it;
      double q = 0.0;
      if (it != lv.end()) q = double((*it).get_double());
      out.bids[out.bid_count++] = {p, q};
    }

    out.ask_count = 0;
    for (auto lv : result["asks"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if (it == lv.end()) continue;
      double p = (*it).get_double();
      ++it;
      double q = 0.0;
      if (it != lv.end()) q = double((*it).get_double());
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

}  // namespace gateio_parser
}  // namespace sqc
