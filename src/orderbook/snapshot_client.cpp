#include "snapshot_client.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

OrderbookSnapshot SnapshotClient::FetchSnapshot(std::string_view rest_host,
                                                std::string_view channel_type,
                                                std::string_view symbol,
                                                uint32_t limit) {
  if (rest_host.find("gateio") != std::string_view::npos)
    return FetchGateioSnapshot(channel_type, symbol, limit);
  return FetchBinanceSnapshot(rest_host, symbol, limit);
}

OrderbookSnapshot SnapshotClient::FetchBinanceSnapshot(std::string_view rest_host,
                                                       std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};

  // URL: https://<rest_host>/api/v3/depth?symbol=<symbol>&limit=<limit>
  // rest_host distinguishes spot (api.binance.com) vs futures (fapi.binance.com)
  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Binance depth from {} for {} limit={}",
           rest_host, symbol, limit);

  snapshot.lastUpdateId = 0;
  snapshot.bid_count = 0;
  snapshot.ask_count = 0;
  return snapshot;
}

OrderbookSnapshot SnapshotClient::FetchGateioSnapshot(std::string_view channel_type,
                                                       std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};

  // Gate.io spot and futures share host (api.gateio.ws) but differ in API path:
  //   spot:  /api/v4/spot/order_book?contract=<symbol>&limit=<limit>
  //   perp:  /api/v4/futures/usdt/order_book?contract=<symbol>&limit=<limit>
  const char* api_path = (channel_type == "spot") ? "spot" : "futures/usdt";

  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Gate.io {}/order_book for {} limit={}",
           api_path, symbol, limit);

  snapshot.lastUpdateId = 0;
  snapshot.bid_count = 0;
  snapshot.ask_count = 0;
  return snapshot;
}

}  // namespace sqc
