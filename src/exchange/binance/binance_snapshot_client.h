#pragma once

#include <cstdint>
#include <string_view>

#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace binance {

// Fetches Binance REST API depth snapshot.
// spot: GET /api/v3/depth?symbol=BTCUSDT&limit=1000
// futures: GET /fapi/v1/depth?symbol=BTCUSDT&limit=1000
OrderbookSnapshot FetchSnapshot(std::string_view rest_host,
                                 std::string_view symbol,
                                 uint32_t limit = 1000);

}  // namespace binance
}  // namespace sqc
