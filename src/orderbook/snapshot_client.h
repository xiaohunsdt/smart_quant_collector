#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "orderbook_event.h"

namespace sqc {

// Fetches REST API depth snapshot for orderbook initialization, per spec §2.2
class SnapshotClient {
 public:
  // Fetch full depth snapshot from exchange REST API.
  // Returns OrderbookSnapshot with lastUpdateId and price levels.
  static OrderbookSnapshot FetchSnapshot(std::string_view rest_host,
                                         std::string_view symbol, uint32_t limit = 1000);

  // Binance-specific: GET /api/v3/depth?symbol=BTCUSDT&limit=1000
  static OrderbookSnapshot FetchBinanceSnapshot(std::string_view symbol,
                                                uint32_t limit = 1000);

  // Gate.io-specific
  static OrderbookSnapshot FetchGateioSnapshot(std::string_view symbol,
                                               uint32_t limit = 1000);
};

}  // namespace sqc
