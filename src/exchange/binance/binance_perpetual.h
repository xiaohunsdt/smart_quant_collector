#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_perpetual {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view symbol, EventType event_type);

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id, std::string_view symbol);

}  // namespace binance_perpetual
}  // namespace sqc
