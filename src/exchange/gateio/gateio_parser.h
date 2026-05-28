#pragma once

#include <cstdint>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_parser {

// Unified entry point: parses the document and determines event type internally.
ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view channel_type);

// Low-level parsers (exposed for unit tests)
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

}  // namespace gateio_parser
}  // namespace sqc
