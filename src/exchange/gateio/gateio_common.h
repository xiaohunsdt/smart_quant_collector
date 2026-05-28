#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio {

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id);
bool ParseDepthUpdateEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

// Shared REST snapshot fetch + parse (called by spot/perpetual wrappers).
OrderbookSnapshot FetchSnapshot(std::string_view rest_host, ChannelType channel_type,
                                std::string_view symbol, uint32_t limit = 50);

}  // namespace gateio
}  // namespace sqc
