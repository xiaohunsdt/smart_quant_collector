#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_spot {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view symbol, EventType event_type);

// Internal parsers (exposed for tests)
bool ParsePartialDepth(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id, std::string_view symbol);

}  // namespace binance_spot
}  // namespace sqc
