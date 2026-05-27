#pragma once

#include <cstdint>
#include <map>
#include <string_view>
#include <vector>

#include "orderbook_event.h"

namespace sqc {

// Local Limit Order Book — price to quantity map, per spec 2.2
class LocalLOB {
 public:
  explicit LocalLOB(uint32_t depth_level = 10);

  void ApplySnapshot(const OrderbookSnapshot& snapshot);
  void UpdateDepth(const DepthUpdateEvent& event);
  void ForceAlignWithEvent(const DepthUpdateEvent& event);

  double BestBid() const;
  double BestAsk() const;

  uint64_t last_update_id() const { return last_update_id_; }
  void set_last_update_id(uint64_t id) { last_update_id_ = id; }
  uint32_t depth_level() const { return depth_level_; }

  const std::map<double, double, std::greater<double>>& bids() const { return bids_; }
  const std::map<double, double>& asks() const { return asks_; }

  // Returns top N price levels as vectors (for CSV writing)
  std::vector<PriceLevel> TopBids(uint32_t max_levels = 0) const;
  std::vector<PriceLevel> TopAsks(uint32_t max_levels = 0) const;

 private:
  std::map<double, double, std::greater<double>> bids_;
  std::map<double, double> asks_;
  uint64_t last_update_id_ = 0;
  uint32_t depth_level_;
};

}  // namespace sqc
