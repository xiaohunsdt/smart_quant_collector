#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_spot {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type);

// Internal parsers (exposed for tests)
bool ParsePartialDepth(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

}  // namespace binance_spot

extern const ExchangeAdapter kBinanceSpotAdapter;

}  // namespace sqc
