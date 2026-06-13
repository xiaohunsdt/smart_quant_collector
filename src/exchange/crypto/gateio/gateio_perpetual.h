#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_perpetual {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type);

EventType PeekEventType(simdjson::ondemand::document& doc);

// Exposed for tests
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

}  // namespace gateio_perpetual

extern const ExchangeAdapter kGateioPerpetualAdapter;

}  // namespace sqc
