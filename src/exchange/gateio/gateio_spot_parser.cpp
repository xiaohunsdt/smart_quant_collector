#include "gateio_spot_parser.h"
#include <cstring>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"

namespace sqc {
namespace gateio_spot {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id) {
  ParseResult result;
  try {
    std::string_view channel = doc["channel"].get_string();
    if (channel == "spot.trades") {
      result.type = ParsedType::TICK;
      if (!gateio::ParseTradeEvent(doc, result.tick, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.order_book") {
      result.type = ParsedType::DEPTH;
      if (!ParseDepthEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.order_book_update") {
      result.type = ParsedType::DEPTH;
      if (!gateio::ParseDepthUpdateEvent(doc, result.depth, channel_id))
        result.type = ParsedType::NONE;
    } else if (channel == "spot.book_ticker") {
      result.type = ParsedType::BOOK_TICKER;
      if (!gateio::ParseBookTickerEvent(doc, result.book_ticker, channel_id))
        result.type = ParsedType::NONE;
    }
  } catch (const simdjson::simdjson_error& e) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot ParseMessage: {}", e.what());
  }
  return result;
}

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
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: {}", e.what());
    return false;
  } catch (...) {
    if (auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: unknown error");
    return false;
  }
}

}  // namespace gateio_spot
}  // namespace sqc
