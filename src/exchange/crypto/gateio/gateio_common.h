#pragma once

#include <string_view>

#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio {

// Determine the event type by matching the message's "channel" field against
// `prefix` (e.g. "spot" or "futures").
EventType PeekEventType(simdjson::ondemand::document& doc, std::string_view prefix);

// Parse a book-ticker message.  Identical layout in both spot and perpetual.
// symbol is NOT accepted; ShardParserWorker stamps it after the call.
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out,
                          uint32_t channel_id);

}  // namespace gateio
}  // namespace sqc
