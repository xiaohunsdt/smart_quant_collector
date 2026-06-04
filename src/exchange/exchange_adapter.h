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
    case ChannelType::Spot:   return "spot";
    case ChannelType::Perpetual: return "perpetual";
    case ChannelType::Futures: return "futures";
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

// Single adapter per (exchange, channel_type) — no runtime branching.
struct ExchangeAdapter {
  std::string_view name;
  ChannelType channel_type;
  std::string_view rest_host;

  std::vector<SubscriptionGroup> (*build_subscribes)(std::string_view symbol, uint32_t depth_level);

  // Lightweight JSON peek on network thread — reads one field to determine type.
  EventType (*peek_event_type)(simdjson::ondemand::document& doc);

  // Parse: receives pre-determined symbol and event_type. No JSON probing needed.
  ParseResult (*parse)(simdjson::ondemand::document& doc, uint32_t channel_id, std::string_view symbol, EventType event_type);
};

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type);

}  // namespace sqc
