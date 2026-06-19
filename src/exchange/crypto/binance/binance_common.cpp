#include "binance_common.h"

#include <cassert>
#include <chrono>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/json_parse_helpers.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace binance {

EventType PeekEventType(simdjson::ondemand::document& doc) {
  auto e_field = doc.find_field("e");
  if(e_field.error()) {
    // Binance SPOT: @bookTicker and @depth<levels>@100ms omit "e".
    // Probe for "lastUpdateId" to detect partial depth.
    doc.rewind();
    auto lu = doc.find_field("lastUpdateId");
    if(!lu.error()) return EventType::DEPTH;
    // Fallback: bookTicker (best bid "b" is a string)
    return EventType::BOOK_TICKER;
  }

  // get_string() may succeed at the field level but still fail if "e" exists
  // with a non-string value (e.g. {"e":123}); guard before reading the value.
  auto ev_res = e_field.get_string();
  if(ev_res.error()) return EventType::UNKNOWN;
  std::string_view ev = ev_res.value();
  if(ev == "trade" || ev == "aggTrade") return EventType::TICK;
  if(ev == "depthUpdate") return EventType::DEPTH;
  if(ev == "bookTicker") return EventType::BOOK_TICKER;

  return EventType::UNKNOWN;
}

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64());
    out.trade_id = static_cast<uint64_t>(doc["a"].get_int64());
    if(!SvToDouble(doc["p"].get_string(), out.price)) return false;
    if(!SvToDouble(doc["q"].get_string(), out.quantity)) return false;
    out.is_buyer_maker = doc["m"].get_bool();
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance trade parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance trade parse: unknown error");
    return false;
  }
}

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    if(!SvToDouble(doc["b"].get_string(), out.best_bid_price)) return false;
    if(!SvToDouble(doc["B"].get_string(), out.best_bid_qty)) return false;
    if(!SvToDouble(doc["a"].get_string(), out.best_ask_price)) return false;
    if(!SvToDouble(doc["A"].get_string(), out.best_ask_qty)) return false;
    out.exchange_timestamp = MsToUs(static_cast<uint64_t>(doc["E"].get_int64()));
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance bookTicker parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance bookTicker parse: unknown error");
    return false;
  }
}

bool ParseSpotBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  try {
    if(!SvToDouble(doc["b"].get_string(), out.best_bid_price)) return false;
    if(!SvToDouble(doc["B"].get_string(), out.best_bid_qty)) return false;
    if(!SvToDouble(doc["a"].get_string(), out.best_ask_price)) return false;
    if(!SvToDouble(doc["A"].get_string(), out.best_ask_qty)) return false;
    // Binance SPOT @bookTicker carries no timestamp field in its payload (no
    // "E"/"T"). We fall back to the collector's wall-clock (system_clock::now).
    // KNOWN LIMITATION: exchange_timestamp here is the local collection moment,
    // not the exchange's event time — so SPOT bookTicker rows are unsuitable for
    // precise replay/backtesting. (Gate.io bookTicker does carry an exchange
    // "t" field and is unaffected; perpetual bookTicker uses doc["E"].)
    out.exchange_timestamp =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance spot bookTicker parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance spot bookTicker parse: unknown error");
    return false;
  }
}

}  // namespace binance
}  // namespace sqc
