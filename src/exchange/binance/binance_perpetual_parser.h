#pragma once

#include <cstdint>

#include "binance_common.h"
#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_perpetual {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id);
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

}  // namespace binance_perpetual
}  // namespace sqc
