#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson.h"
#include "src/common/string_utils.h"  // SvToDouble — used by ParseQuantity below
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace gateio {

// HTTP headers Gate.io requires on the WebSocket upgrade handshake (shared by
// spot and perpetual). Backing storage for ExchangeAdapter::ws_headers.
inline constexpr HttpHeader kGateioWsHeaders[] = {
    {"X-Gate-Aggregation", "0"},
    {"X-Gate-Size-Decimal", "1"},
};

// Determine the event type by matching the message's "channel" field against
// `prefix` (e.g. "spot" or "futures").
EventType PeekEventType(simdjson::ondemand::document& doc, std::string_view prefix);

// Parse a book-ticker message.  Identical layout in both spot and perpetual.
// symbol is NOT accepted; ShardParserWorker stamps it after the call.
bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id);

// Advance the cursor past the optional "time"/"time_ms" envelope fields that
// Gate.io prepends to every message.  Both are read-and-discarded so the
// downstream fields remain reachable under simdjson's on-demand (forward-only)
// model.  Tolerates a missing "time_ms" (older messages omit it).
inline void SkipTimeEnvelope(simdjson::ondemand::document& doc) {
  (void)doc["time"].get_uint64();
  try {
    (void)doc["time_ms"].get_uint64();
  } catch(...) {
  }
}

// Read a quantity that Gate.io encodes either as a JSON string ("138237") or a
// bare integer (138237).  Tries string first; on failure falls back to int64.
// Returns the parsed value (0.0 on total failure, matching pre-refactor code).
inline double ParseQuantity(simdjson::ondemand::value field) {
  double qty = 0.0;
  std::string_view sv;
  if(field.get_string().get(sv) == simdjson::SUCCESS)
    SvToDouble(sv, qty);
  else
    qty = static_cast<double>(field.get_int64());
  return qty;
}

}  // namespace gateio
}  // namespace sqc
