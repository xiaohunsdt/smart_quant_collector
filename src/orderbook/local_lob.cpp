#include "local_lob.h"

namespace sqc {

LocalLOB::LocalLOB(uint32_t depth_level) : depth_level_(depth_level) {}

void LocalLOB::ApplySnapshot(const OrderbookSnapshot& snapshot) {
  bid_count_ = std::min(snapshot.bid_count, kMaxOrderbookLevels);
  ask_count_ = std::min(snapshot.ask_count, kMaxOrderbookLevels);
  last_update_id_ = snapshot.lastUpdateId;

  for (uint32_t i = 0; i < bid_count_; ++i) bids_[i] = snapshot.bids[i];
  for (uint32_t i = 0; i < ask_count_; ++i) asks_[i] = snapshot.asks[i];
}

void LocalLOB::UpdateSide(PriceLevel* levels, uint32_t& count, uint32_t capacity,
                          double price, double qty, bool is_bid) {
  // Linear search: optimal for small arrays (typical depth 5-20)
  uint32_t idx = 0;
  for (; idx < count; ++idx) {
    if (levels[idx].price == price) {
      // Exact match: update or remove
      if (qty == 0.0) {
        // Remove by shifting
        for (uint32_t j = idx; j + 1 < count; ++j)
          levels[j] = levels[j + 1];
        --count;
      } else {
        levels[idx].quantity = qty;
      }
      return;
    }
    // For bids: descending order, so stop when price > existing
    // For asks: ascending order, so stop when price < existing
    if (is_bid) {
      if (price > levels[idx].price) break;
    } else {
      if (price < levels[idx].price) break;
    }
  }

  if (qty == 0.0) return;  // nothing to insert

  // Insert at idx, shift right
  if (count < capacity) {
    for (uint32_t j = count; j > idx; --j)
      levels[j] = levels[j - 1];
    levels[idx] = {price, qty};
    ++count;
  } else if (idx < capacity) {
    // At capacity: shift right, drop the worst level (last element)
    for (uint32_t j = capacity - 1; j > idx; --j)
      levels[j] = levels[j - 1];
    levels[idx] = {price, qty};
  }
  // If idx == capacity, new price is worse than all existing levels — drop it
}

void LocalLOB::UpdateDepth(const DepthUpdateEvent& event) {
  last_update_id_ = event.last_update_id;

  for (uint32_t i = 0; i < event.bid_count; ++i)
    UpdateSide(bids_, bid_count_, kMaxOrderbookLevels,event.bids[i].price, event.bids[i].quantity, true);
  for (uint32_t i = 0; i < event.ask_count; ++i)
    UpdateSide(asks_, ask_count_, kMaxOrderbookLevels,event.asks[i].price, event.asks[i].quantity, false);
}

void LocalLOB::ForceAlignWithEvent(const DepthUpdateEvent& event) {
  bid_count_ = 0;
  ask_count_ = 0;
  last_update_id_ = event.last_update_id;

  for (uint32_t i = 0; i < event.bid_count; ++i)
    bids_[i] = event.bids[i];
  bid_count_ = event.bid_count;

  for (uint32_t i = 0; i < event.ask_count; ++i)
    asks_[i] = event.asks[i];
  ask_count_ = event.ask_count;
}

double LocalLOB::BestBid() const {
  if (bid_count_ == 0) return 0.0;
  return bids_[0].price;
}

double LocalLOB::BestAsk() const {
  if (ask_count_ == 0) return 0.0;
  return asks_[0].price;
}

uint32_t LocalLOB::TopBids(PriceLevel* out, uint32_t max_levels) const {
  if (max_levels == 0) max_levels = depth_level_;
  uint32_t n = (max_levels < bid_count_) ? max_levels : bid_count_;
  for (uint32_t i = 0; i < n; ++i) out[i] = bids_[i];
  return n;
}

uint32_t LocalLOB::TopAsks(PriceLevel* out, uint32_t max_levels) const {
  if (max_levels == 0) max_levels = depth_level_;
  uint32_t n = (max_levels < ask_count_) ? max_levels : ask_count_;
  for (uint32_t i = 0; i < n; ++i) out[i] = asks_[i];
  return n;
}

}  // namespace sqc
