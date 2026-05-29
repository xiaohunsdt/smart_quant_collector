#pragma once

#include <cstdint>
#include <string_view>

#include "orderbook_event.h"

namespace sqc {

// Local Limit Order Book — flat sorted array, per spec 2.2
// Replaces std::map to eliminate per-update heap allocation (F07).
class LocalLOB {
 public:
  explicit LocalLOB(uint32_t depth_level = 10);

  void ApplySnapshot(const OrderbookSnapshot& snapshot);
  void UpdateDepth(const DepthUpdateEvent& event);
  void ForceAlignWithEvent(const DepthUpdateEvent& event);

  // Update best bid/ask from bookTicker. Returns true if best prices changed.
  bool UpdateBestPrice(double best_bid_price, double best_bid_qty,
                       double best_ask_price, double best_ask_qty);

  double BestBid() const;
  double BestAsk() const;

  double BestBidVolume() const;
  double BestAskVolume() const;

  uint64_t last_update_id() const { return last_update_id_; }
  void set_last_update_id(uint64_t id) { last_update_id_ = id; }
  uint32_t depth_level() const { return depth_level_; }

  // Writes top N price levels into caller-provided array (zero-allocation).
  // Returns number of levels written.
  uint32_t TopBids(PriceLevel* out, uint32_t max_levels) const;
  uint32_t TopAsks(PriceLevel* out, uint32_t max_levels) const;

  uint32_t BidCount() const { return bid_count_; }
  uint32_t AskCount() const { return ask_count_; }

 private:
  // Update side (bid/ask) maintaining sorted order.
  // Side == true for bids (descending), false for asks (ascending).
  static void UpdateSide(PriceLevel* levels, uint32_t& count, uint32_t capacity,
                         double price, double qty, bool is_bid);

  PriceLevel bids_[kMaxOrderbookLevels];
  PriceLevel asks_[kMaxOrderbookLevels];
  uint32_t bid_count_ = 0;
  uint32_t ask_count_ = 0;
  uint64_t last_update_id_ = 0;
  uint32_t depth_level_;
};

}  // namespace sqc
