#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_perpetual {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type);

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

}  // namespace binance_perpetual

extern const ExchangeAdapter kBinancePerpetualAdapter;

}  // namespace sqc
