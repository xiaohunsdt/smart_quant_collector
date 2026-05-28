#include "binance_common.h"
#include <cstring>
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"

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

}  // namespace binance
}  // namespace sqc
