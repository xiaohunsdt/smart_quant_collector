#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance {

// Shared parser functions.  symbol is NOT accepted here; the caller
// (ShardParserWorker) stamps it into the result after parse returns.
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);
bool ParseSpotBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

// Lightweight peek: reads "e" field to determine EventType.
EventType PeekEventType(simdjson::ondemand::document& doc);

}  // namespace binance
}  // namespace sqc
