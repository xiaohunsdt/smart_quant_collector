#pragma once

#include <cstdint>
#include <string_view>

#include "orderbook_event.h"

namespace sqc {

// Fetches REST API depth snapshot for orderbook initialization, per spec §2.2
class SnapshotClient {
 public:
  // Fetch full depth snapshot from exchange REST API.
  // channel_type distinguishes spot vs perpetual for URL path construction (e.g. Gate.io).
  static OrderbookSnapshot FetchSnapshot(std::string_view rest_host,
                                         std::string_view channel_type,
                                         std::string_view symbol, uint32_t limit = 1000);

  // Binance-specific: GET /api/v3/depth?symbol=BTCUSDT&limit=1000
  //          or: GET /fapi/v1/depth?symbol=BTCUSDT&limit=1000
  static OrderbookSnapshot FetchBinanceSnapshot(std::string_view rest_host,
                                                std::string_view symbol,
                                                uint32_t limit = 1000);

  // Gate.io-specific: GET /api/v4/spot/order_book?contract=BTC_USDT
  //              or: GET /api/v4/futures/usdt/order_book?contract=BTC_USDT
  static OrderbookSnapshot FetchGateioSnapshot(std::string_view channel_type,
                                               std::string_view symbol,
                                               uint32_t limit = 1000);
};

}  // namespace sqc
