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
    // IMPORTANT: ondemand requires forward-only field access — must match JSON order.
    // JSON order: time, time_ms, channel, event, result

    // 1. time (int64 seconds) — comes before "result" in JSON
    out.exchange_timestamp = doc["time"].get_uint64() * 1'000'000ULL;

    // 2. event — comes before "result"
    std::string_view ev = doc["event"].get_string();
    if (ev == "subscribe") return false;

    // 3. result array — last field
    auto arr = doc["result"].get_array();
    auto it = arr.begin();
    if (it == arr.end()) return false;
    auto item = *it;

    // Gate.io ticker: all price/qty fields are STRINGS.
    // simdjson string_view is NOT null-terminated — copy to stack buffer first.
    char num_buf[32];
    auto sv_to_double = [&](std::string_view sv) -> double {
      size_t n = std::min(sv.size(), sizeof(num_buf) - 1);
      std::memcpy(num_buf, sv.data(), n);
      num_buf[n] = '\0';
      return std::strtod(num_buf, nullptr);
    };

    out.price = sv_to_double(item["last"].get_string());
    out.quantity = sv_to_double(item["volume_24h_base"].get_string());

    // t is an integer (ms) — trade timestamp
    out.exchange_timestamp = item["t"].get_uint64() * 1000ULL;
    out.trade_id = out.exchange_timestamp;

    out.channel_id = channel_id;

    std::string_view sym = item["contract"].get_string();
    size_t sym_len = std::min(sym.size(), sizeof(out.symbol) - 1);
    std::memcpy(out.symbol, sym.data(), sym_len);
    out.symbol[sym_len] = '\0';
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
    // Forward-only: access "time" then "event" then "result" (JSON order)
    out.exchange_timestamp = doc["time"].get_uint64() * 1'000'000ULL;

    std::string_view ev = doc["event"].get_string();
    if (ev != "update") return false;

    auto result = doc["result"];
    out.U = result["id"].get_uint64(); out.u = out.U;
    out.channel_id = channel_id;
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
