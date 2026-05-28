#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

// Unified parsed-event types — shared by ALL exchanges
enum class ParsedType { NONE, TICK, DEPTH, BOOK_TICKER };

struct ParseResult {
  ParsedType type = ParsedType::NONE;
  TickData tick{};
  DepthUpdateEvent depth{};
  BookTickerEvent book_ticker{};
};

enum class ChannelType : uint8_t { Spot, Perpetual };

inline ChannelType ParseChannelType(std::string_view s) {
  if (s == "spot") return ChannelType::Spot;
  if (s == "perpetual") return ChannelType::Perpetual;
  throw std::invalid_argument(std::string("unknown channel_type: ") + std::string(s));
}

inline const char* ChannelTypeName(ChannelType t) {
  switch (t) {
    case ChannelType::Spot: return "spot";
    case ChannelType::Perpetual: return "perpetual";
  }
  return "unknown";
}

// Exchange-specific endpoints for one channel type.
struct ExchangeEndpoints {
  std::string_view ws_url;
  std::string_view rest_host;
};

// Function-pointer-based adapter — no vtable, no heap allocation.
// All exchange-specific behavior is encapsulated here.
struct ExchangeAdapter {
  std::string_view name;
  bool snapshot_mode = false;

  // Endpoints keyed by channel type — spot vs perpetual is explicit.
  ExchangeEndpoints spot;
  ExchangeEndpoints perpetual;

  const ExchangeEndpoints& endpoints(ChannelType channel_type) const {
    return channel_type == ChannelType::Spot ? spot : perpetual;
  }

  // Build WebSocket subscribe messages: vector of (JSON_payload, delay_ms).
  // 0 delay = send immediately; >0 = schedule after N ms.
  std::vector<std::pair<std::string, uint32_t>> (*build_subscribes)(ChannelType channel_type, std::string_view symbol, uint32_t depth_level);

  // Hot-path: parse a WebSocket frame into a ParseResult.
  ParseResult (*parse)(simdjson::ondemand::document& doc, uint32_t channel_id, ChannelType channel_type);

  // Cold-path: fetch REST orderbook snapshot on a background thread.
  OrderbookSnapshot (*fetch_snapshot)(std::string_view rest_host, ChannelType channel_type, std::string_view symbol);
};

// Returns nullptr for unknown exchange names.
const ExchangeAdapter* GetAdapter(std::string_view exchange_name);

}  // namespace sqc
