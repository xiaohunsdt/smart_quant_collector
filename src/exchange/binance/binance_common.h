#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance {

// Shared parser functions — identical for spot and perpetual.
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

// Spot-specific bookTicker parser: Binance spot @bookTicker omits "e" and "E" fields.
// exchange_timestamp is computed from system_clock since the exchange provides none.
bool ParseSpotBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

// Shared REST snapshot fetch + parse (called by spot/perpetual wrappers).
OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol, uint32_t limit = 1000);

}  // namespace binance
}  // namespace sqc
