#include "gateio_common.h"

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/json_parse_helpers.h"
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
    SkipTimeEnvelope(doc);
    std::string_view ev = doc["event"].get_string();
    if(ev != "update") return false;
    auto result = doc["result"];
    out.exchange_timestamp = MsToUs(result["t"].get_uint64());
    if(!SvToDouble(result["b"].get_string(), out.best_bid_price)) return false;
    out.best_bid_qty = ParseQuantity(result["B"]);
    if(!SvToDouble(result["a"].get_string(), out.best_ask_price)) return false;
    out.best_ask_qty = ParseQuantity(result["A"]);
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
