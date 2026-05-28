#include "binance_parser.h"
#include <cstring>
#include <string>
#include <limits>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "src/common/tick_data.h"

namespace sqc {
namespace binance_parser {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id) {
  ParseResult result;
  try {
    std::string_view event_type = doc["e"].get_string();
    if (event_type == "trade" || event_type == "aggTrade") {
      result.type = ParsedType::TICK;
      if (!ParseTradeEvent(doc, result.tick, channel_id)) {
        result.type = ParsedType::NONE;
      }
    } else if (event_type == "depthUpdate") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id)) {
        result.type = ParsedType::NONE;
      }
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance ParseMessage: {}", e.what());
  }
  return result;
}

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64() * 1000);
    out.price = SvToDouble(doc["p"].get_string());
    out.quantity = SvToDouble(doc["q"].get_string());
    out.trade_id = static_cast<uint64_t>(doc["t"].get_int64());
    out.is_buyer_maker = doc["m"].get_bool();
    out.channel_id = channel_id;
    std::string_view sym = doc["s"].get_string();
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

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    out.U = static_cast<uint64_t>(doc["U"].get_int64());
    out.u = static_cast<uint64_t>(doc["u"].get_int64());
    out.channel_id = channel_id;
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64() * 1000);
    std::string_view sym = doc["s"].get_string();
    std::memcpy(out.symbol, sym.data(), std::min(sym.size(), sizeof(out.symbol) - 1));

    out.bid_count = 0;
    for (auto bid_level : doc["b"]) {
      if (out.bid_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = bid_level.begin();
      if (it != bid_level.end()) { p = SvToDouble((*it).get_string()); }
      ++it;
      if (it != bid_level.end()) { q = SvToDouble((*it).get_string()); }
      out.bids[out.bid_count++] = {p, q};
    }

    out.ask_count = 0;
    for (auto ask_level : doc["a"]) {
      if (out.ask_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = ask_level.begin();
      if (it != ask_level.end()) { p = SvToDouble((*it).get_string()); }
      ++it;
      if (it != ask_level.end()) { q = SvToDouble((*it).get_string()); }
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance depth parse: unknown error");
    return false;
  }
}

}  // namespace binance_parser
}  // namespace sqc
