#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance {

// Shared parser functions — symbol is passed in, not read from JSON.
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id, std::string_view symbol);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id, std::string_view symbol);
bool ParseSpotBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id, std::string_view symbol);

// Lightweight peek: reads "e" field to determine EventType.
EventType PeekEventType(simdjson::ondemand::document& doc);

}  // namespace binance
}  // namespace sqc
