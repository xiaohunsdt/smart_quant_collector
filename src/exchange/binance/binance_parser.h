#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance_parser {

bool ParseTradeEvent(simdjson::ondemand::document& doc, struct TickData& out, uint32_t channel_id);

// Returns filled DepthUpdateEvent with bids/asks arrays
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

}  // namespace binance_parser
}  // namespace sqc
