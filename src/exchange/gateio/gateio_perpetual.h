#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_perpetual {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id);
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol);

}  // namespace gateio_perpetual
}  // namespace sqc
