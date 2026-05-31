#pragma once

#include <cstdint>

namespace sqc {

constexpr uint32_t kMaxOrderbookLevels = 100;

struct PriceLevel {
  double price;
  double quantity;
};

struct DepthUpdateEvent {
  uint64_t last_update_id = 0;        // last update ID in this event (Binance: u)
  uint32_t channel_id;
  uint64_t exchange_timestamp = 0;
  uint64_t local_timestamp = 0;
  char symbol[12] = {};

  PriceLevel bids[kMaxOrderbookLevels];
  PriceLevel asks[kMaxOrderbookLevels];
  uint32_t bid_count = 0;
  uint32_t ask_count = 0;
};

struct BookTickerEvent {
  uint32_t channel_id;
  uint64_t exchange_timestamp = 0;
  uint64_t local_timestamp = 0;
  char symbol[12] = {};
  double best_bid_price = 0.0;
  double best_bid_qty = 0.0;
  double best_ask_price = 0.0;
  double best_ask_qty = 0.0;
};

}  // namespace sqc
