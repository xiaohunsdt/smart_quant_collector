#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_parser {

bool ParseTradeEvent(simdjson::ondemand::document& doc, struct TickData& out, uint32_t channel_id);

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

}  // namespace gateio_parser
}  // namespace sqc
