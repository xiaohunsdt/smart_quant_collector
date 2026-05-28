#include "binance_spot_parser.h"
#include <cstring>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"

namespace sqc {
namespace binance_spot {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id) {
  ParseResult result;
  auto e_field = doc.find_field("e");
  if (e_field.error()) return result;
  try {
    std::string_view event_type = e_field.get_string();
    if (event_type == "trade" || event_type == "aggTrade") {
      result.type = ParsedType::TICK;
      if (!binance::ParseTradeEvent(doc, result.tick, channel_id))
        result.type = ParsedType::NONE;
    } else if (event_type == "depthUpdate") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (event_type == "bookTicker") {
      result.type = ParsedType::BOOK_TICKER;
      if (!binance::ParseBookTickerEvent(doc, result.book_ticker, channel_id))
        result.type = ParsedType::NONE;
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance spot ParseMessage: {}", e.what());
  }
  return result;
}

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64() * 1000);
    std::string_view sym = doc["s"].get_string();
    out.first_update_id = static_cast<uint64_t>(doc["U"].get_int64());
    out.last_update_id = static_cast<uint64_t>(doc["u"].get_int64());
    // Spot depthUpdate has no "pu" field — compute from first_update_id.
    out.prev_last_update_id = out.first_update_id > 0 ? out.first_update_id - 1 : 0;
    out.channel_id = channel_id;
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
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance spot depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* log = GetLogger()) LOG_ERROR(log, "Binance spot depth parse: unknown error");
    return false;
  }
}

}  // namespace binance_spot
}  // namespace sqc
