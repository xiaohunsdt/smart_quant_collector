#pragma once

#include <cstdint>
#include <string_view>

#include "gateio_common.h"
#include "simdjson.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio_perpetual {

ParseResult ParseMessage(simdjson::ondemand::document& doc, uint32_t channel_id);
bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id);

OrderbookSnapshot FetchSnapshot(std::string_view rest_host, std::string_view symbol);

}  // namespace gateio_perpetual
}  // namespace sqc
