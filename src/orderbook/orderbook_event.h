#pragma once

#include <cstdint>

namespace sqc {

constexpr uint32_t kMaxOrderbookLevels = 100;

// Stack arrays in dolphindb_client.cpp:TableInsertOrderbook() allocate
// kMaxOrderbookLevels * 4 * sizeof(double) = 3.2 KB at kMaxOrderbookLevels=100.
// Increasing this beyond 200 risks stack overflow on the hot path.
static_assert(kMaxOrderbookLevels <= 200,
              "kMaxOrderbookLevels must fit in stack (~6 KB); "
              "increase only after auditing DolphinDB insert path stack usage");

struct PriceLevel {
  double price;
  double quantity;
};

struct DepthUpdateEvent {
  uint64_t last_update_id = 0;  // last update ID in this event (Binance: u)
  uint32_t channel_id;
  uint64_t exchange_timestamp = 0;
  uint64_t local_diff = 0;
  char symbol[12] = {};  // max 11 chars + null (same constraint as TickData::symbol)

  PriceLevel bids[kMaxOrderbookLevels];
  PriceLevel asks[kMaxOrderbookLevels];
  uint32_t bid_count = 0;
  uint32_t ask_count = 0;
};

struct BookTickerEvent {
  uint32_t channel_id;
  uint64_t exchange_timestamp = 0;
  uint64_t local_diff = 0;
  char symbol[12] = {};  // max 11 chars + null (same constraint as TickData::symbol)
  double best_bid_price = 0.0;
  double best_bid_qty = 0.0;
  double best_ask_price = 0.0;
  double best_ask_qty = 0.0;
};

}  // namespace sqc
