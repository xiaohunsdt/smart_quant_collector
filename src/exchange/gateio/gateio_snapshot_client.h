#pragma once

#include <cstdint>
#include <string_view>

#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio {

// Fetches Gate.io REST API depth snapshot.
// spot: GET /api/v4/spot/order_book?currency_pair=BTC_USDT&limit=50&with_id=true
// futures: GET /api/v4/futures/usdt/order_book?contract=BTC_USDT&limit=50
OrderbookSnapshot FetchSnapshot(std::string_view rest_host,
                                 ChannelType channel_type,
                                 std::string_view symbol,
                                 uint32_t limit = 50);

}  // namespace gateio
}  // namespace sqc
