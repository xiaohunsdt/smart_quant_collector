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

// Single adapter per (exchange, channel_type) — no runtime branching.
struct ExchangeAdapter {
  std::string_view name;
  ChannelType channel_type;
  bool snapshot_mode = false;
  std::string_view ws_url;
  std::string_view rest_host;

  std::vector<std::pair<std::string, uint32_t>> (*build_subscribes)(std::string_view symbol, uint32_t depth_level);
  ParseResult (*parse)(simdjson::ondemand::document& doc, uint32_t channel_id);
  OrderbookSnapshot (*fetch_snapshot)(std::string_view rest_host, std::string_view symbol);
};

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type);

}  // namespace sqc
