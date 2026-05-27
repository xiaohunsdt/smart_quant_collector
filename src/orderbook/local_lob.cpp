#include "local_lob.h"

namespace sqc {

LocalLOB::LocalLOB(uint32_t depth_level) : depth_level_(depth_level) {}

void LocalLOB::ApplySnapshot(const OrderbookSnapshot& snapshot) {
  bids_.clear();
  asks_.clear();
  last_update_id_ = snapshot.lastUpdateId;

  for (uint32_t i = 0; i < snapshot.bid_count; ++i)
    bids_[snapshot.bids[i].price] = snapshot.bids[i].quantity;
  for (uint32_t i = 0; i < snapshot.ask_count; ++i)
    asks_[snapshot.asks[i].price] = snapshot.asks[i].quantity;
}

void LocalLOB::UpdateDepth(const DepthUpdateEvent& event) {
  last_update_id_ = event.u;

  for (uint32_t i = 0; i < event.bid_count; ++i) {
    if (event.bids[i].quantity == 0.0)
      bids_.erase(event.bids[i].price);
    else
      bids_[event.bids[i].price] = event.bids[i].quantity;
  }
  for (uint32_t i = 0; i < event.ask_count; ++i) {
    if (event.asks[i].quantity == 0.0)
      asks_.erase(event.asks[i].price);
    else
      asks_[event.asks[i].price] = event.asks[i].quantity;
  }
}

void LocalLOB::ForceAlignWithEvent(const DepthUpdateEvent& event) {
  bids_.clear();
  asks_.clear();
  last_update_id_ = event.u;

  for (uint32_t i = 0; i < event.bid_count; ++i)
    bids_[event.bids[i].price] = event.bids[i].quantity;
  for (uint32_t i = 0; i < event.ask_count; ++i)
    asks_[event.asks[i].price] = event.asks[i].quantity;
}

double LocalLOB::BestBid() const {
  if (bids_.empty()) return 0.0;
  return bids_.begin()->first;
}

double LocalLOB::BestAsk() const {
  if (asks_.empty()) return 0.0;
  return asks_.begin()->first;
}

std::vector<PriceLevel> LocalLOB::TopBids(uint32_t max_levels) const {
  if (max_levels == 0) max_levels = depth_level_;
  std::vector<PriceLevel> result;
  result.reserve(max_levels);
  uint32_t count = 0;
  for (const auto& [price, qty] : bids_) {
    if (count >= max_levels) break;
    result.push_back({price, qty});
    ++count;
  }
  return result;
}

std::vector<PriceLevel> LocalLOB::TopAsks(uint32_t max_levels) const {
  if (max_levels == 0) max_levels = depth_level_;
  std::vector<PriceLevel> result;
  result.reserve(max_levels);
  uint32_t count = 0;
  for (const auto& [price, qty] : asks_) {
    if (count >= max_levels) break;
    result.push_back({price, qty});
    ++count;
  }
  return result;
}

}  // namespace sqc
