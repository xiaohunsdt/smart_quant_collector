#pragma once

#include <cstdint>

namespace sqc {

constexpr uint32_t kMaxOrderbookLevels = 100;

struct PriceLevel {
  double price;
  double quantity;
};

struct DepthUpdateEvent {
  uint64_t U;  // first update ID in this event
  uint64_t u;  // last update ID in this event
  uint32_t channel_id;
  uint64_t exchange_timestamp = 0;
  uint64_t local_timestamp = 0;
  char symbol[12] = {};

  PriceLevel bids[kMaxOrderbookLevels];
  PriceLevel asks[kMaxOrderbookLevels];
  uint32_t bid_count = 0;
  uint32_t ask_count = 0;
};

struct OrderbookSnapshot {
  uint64_t lastUpdateId;

  PriceLevel bids[kMaxOrderbookLevels];
  PriceLevel asks[kMaxOrderbookLevels];
  uint32_t bid_count = 0;
  uint32_t ask_count = 0;
};

}  // namespace sqc
