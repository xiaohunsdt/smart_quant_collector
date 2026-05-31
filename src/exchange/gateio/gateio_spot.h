#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_spot {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id,
                 std::string_view symbol, EventType event_type);

EventType PeekEventType(simdjson::ondemand::document& doc);

// Exposed for tests
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out,
                     uint32_t channel_id, std::string_view symbol);
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out,
                     uint32_t channel_id, std::string_view symbol);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out,
                          uint32_t channel_id, std::string_view symbol);

}  // namespace gateio_spot
}  // namespace sqc
