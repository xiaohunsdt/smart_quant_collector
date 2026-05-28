#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio {

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

}  // namespace gateio
}  // namespace sqc
