#include "gateio_common.h"

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace gateio {

EventType PeekEventType(simdjson::ondemand::document& doc, std::string_view prefix) {
  auto ch = doc["channel"].get_string();
  if(ch.error()) return EventType::UNKNOWN;
  std::string_view channel = ch.value_unsafe();

  // Channel names: "<prefix>.trades", "<prefix>.order_book", "<prefix>.book_ticker"
  if(channel.size() <= prefix.size() + 1) return EventType::UNKNOWN;
  if(channel.substr(0, prefix.size()) != prefix) return EventType::UNKNOWN;
  std::string_view suffix = channel.substr(prefix.size() + 1);  // skip "prefix."

  if(suffix == "trades") return EventType::TICK;
  if(suffix == "order_book") return EventType::DEPTH;
  if(suffix == "book_ticker") return EventType::BOOK_TICKER;
  return EventType::UNKNOWN;
}

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev != "update") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    if(!SvToDouble(result["b"].get_string(), out.best_bid_price)) return false;
    auto bid_sz = result["B"].get_string();
    if(bid_sz.error() == simdjson::SUCCESS)
      SvToDouble(bid_sz.value_unsafe(), out.best_bid_qty);
    else
      out.best_bid_qty = static_cast<double>(result["B"].get_int64());
    if(!SvToDouble(result["a"].get_string(), out.best_ask_price)) return false;
    auto ask_sz = result["A"].get_string();
    if(ask_sz.error() == simdjson::SUCCESS)
      SvToDouble(ask_sz.value_unsafe(), out.best_ask_qty);
    else
      out.best_ask_qty = static_cast<double>(result["A"].get_int64());
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io bookTicker parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io bookTicker parse: unknown error");
    return false;
  }
}

}  // namespace gateio
}  // namespace sqc
