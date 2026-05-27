#include "snapshot_client.h"

#include <sstream>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

OrderbookSnapshot SnapshotClient::FetchSnapshot(std::string_view /*rest_host*/,
                                                std::string_view symbol,
                                                uint32_t limit) {
  // Default: try Binance format
  return FetchBinanceSnapshot(symbol, limit);
}

OrderbookSnapshot SnapshotClient::FetchBinanceSnapshot(std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};

  // In production, this would use Boost.Beast HTTP client with SSL.
  // For now, provide a stub that logs the intent.
  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Binance depth for {} limit={}", symbol, limit);

  // Stub: return empty snapshot (real implementation will use HTTP client)
  snapshot.lastUpdateId = 0;
  snapshot.bid_count = 0;
  snapshot.ask_count = 0;
  return snapshot;
}

OrderbookSnapshot SnapshotClient::FetchGateioSnapshot(std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};
  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Gate.io depth for {} limit={}", symbol, limit);

  snapshot.lastUpdateId = 0;
  snapshot.bid_count = 0;
  snapshot.ask_count = 0;
  return snapshot;
}

}  // namespace sqc
