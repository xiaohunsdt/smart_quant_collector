#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

namespace rithmic {
class RithmicProcessManager;
}

enum class ParsedType { NONE, TICK, DEPTH, BOOK_TICKER };

enum class EventType : uint8_t { UNKNOWN, TICK, DEPTH, BOOK_TICKER };

struct ParseResult {
  ParsedType type = ParsedType::NONE;
  TickData tick{};
  DepthUpdateEvent depth{};
  BookTickerEvent book_ticker{};
};

enum class ChannelType : uint8_t { Spot, Perpetual, Futures };

inline ChannelType ParseChannelType(std::string_view s) {
  if(s == "spot") return ChannelType::Spot;
  if(s == "perpetual") return ChannelType::Perpetual;
  if(s == "futures") return ChannelType::Futures;
  throw std::invalid_argument(std::string("unknown channel_type: ") + std::string(s));
}

inline const char* ChannelTypeName(ChannelType t) {
  switch(t) {
    case ChannelType::Spot:
      return "spot";
    case ChannelType::Perpetual:
      return "perpetual";
    case ChannelType::Futures:
      return "futures";
  }
  return "unknown";
}

struct SubscriptionMessage {
  std::string payload;
  uint32_t delay_ms = 0;
  EventType event_type = EventType::TICK;
};

// A group of subscriptions sharing the same WebSocket connection.
struct SubscriptionGroup {
  std::string ws_url;
  std::vector<SubscriptionMessage> messages;
};

// A custom HTTP header injected during the WebSocket upgrade handshake.
// Exchanged only at connection setup time (not on the data hot path), so
// string_view + span is fine — the backing storage lives in a static constexpr
// array on each adapter (see e.g. kGateioSpotAdapter).
struct HttpHeader {
  std::string_view name;
  std::string_view value;
};

// Single adapter per (exchange, channel_type) — no runtime branching.
struct ExchangeAdapter {
  std::string_view name;
  ChannelType channel_type;
  std::string_view rest_host;

  std::vector<SubscriptionGroup> (*build_subscribes)(std::string_view symbol, uint32_t depth_level);

  // Lightweight JSON peek on network thread — reads one field to determine type.
  EventType (*peek_event_type)(simdjson::ondemand::document& doc);

  // Parse: channel_id and event_type are pre-determined; no JSON probing needed.
  // symbol is NOT passed here — ShardParserWorker stamps it from RawMessage.symbol
  // after the call, keeping parse functions stateless and allocation-free.
  ParseResult (*parse)(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type);

  // Custom HTTP headers to add during the WebSocket upgrade handshake (vendor
  // quirks, e.g. Gate.io's X-Gate-Aggregation). Consumed once per connection by
  // SymbolChannel. Defaults to an empty span — most exchanges need none.
  std::span<const HttpHeader> ws_headers;
};

/// Create and start the Rithmic cross-process pipeline.
/// Returns nullptr if no futures exchanges are configured (Setup() returns false).
/// The caller owns the returned manager and must call Shutdown() before destruction.
std::unique_ptr<rithmic::RithmicProcessManager> CreateRithmicManager(std::string_view config_path);

}  // namespace sqc
