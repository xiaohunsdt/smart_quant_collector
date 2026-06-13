#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>

#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicOrderBook — per-symbol order book maintained inside the gateway.
//
// Rithmic API delivers order-book data through four callback types:
//   LimitOrderBook(MD_IMAGE_CB)   → full snapshot (rebuild)
//   LimitOrderBook(MD_UPDATE_CB)  → batch incremental update
//   BidQuote / AskQuote           → single-level update
//   EndQuote                      → multi-part sequence terminator
//
// Multi-level atomic updates use UPDATE_TYPE_BEGIN/MIDDLE/END markers.
// The book buffers levels during BEGIN→MIDDLE→END and applies them
// atomically on END (or EndQuote).  After every completed update a
// consistent DepthUpdateEvent snapshot is pushed to the SHM queue.
//
// All arrays are fixed-size at compile time — zero heap allocation on
// the hot path.  Bids are sorted descending (best first), asks ascending.
// ============================================================================

// RApi update-type constants (copied from RApiPlus.h to avoid pulling in
// the full RApi header from a pure data-structure file).
inline constexpr int kUpdateTypeUndefined  = 0;
inline constexpr int kUpdateTypeSolo       = 1;
inline constexpr int kUpdateTypeBegin      = 2;
inline constexpr int kUpdateTypeMiddle     = 3;
inline constexpr int kUpdateTypeEnd        = 4;
inline constexpr int kUpdateTypeClear      = 5;
inline constexpr int kUpdateTypeAggregated = 6;

// RApi iType constants.
inline constexpr int kMdImageCb  = 1;
inline constexpr int kMdUpdateCb = 2;
inline constexpr int kMdHistoryCb = 3;

class RithmicOrderBook {
 public:
  RithmicOrderBook() = default;

  // ---- Identity ----

  void Init(uint32_t channel_id, const char* symbol) noexcept {
    channel_id_ = channel_id;
    size_t n = 0;
    while (n < 11 && symbol[n] != '\0') {
      symbol_[n] = symbol[n];
      ++n;
    }
    symbol_[n] = '\0';
  }

  uint32_t channel_id() const noexcept { return channel_id_; }
  const char* symbol() const noexcept { return symbol_; }

  // ---- Spinlock ----

  void Lock() noexcept {
    while (lock_.test_and_set(std::memory_order_acquire)) {
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
      __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
      __asm__ volatile("yield");
#endif
    }
  }

  void Unlock() noexcept {
    lock_.clear(std::memory_order_release);
  }

  // ---- Full-image replacement (LimitOrderBook MD_IMAGE_CB) ----

  /// Replace one side of the book entirely.  Rithmic delivers sorted arrays.
  void ApplyImage(const double* prices, const long long* sizes,
                  int count, bool is_ask) noexcept {
    auto& levels = is_ask ? asks_ : bids_;
    auto& n     = is_ask ? ask_count_ : bid_count_;

    n = 0;
    uint32_t max_levels = sqc::kMaxOrderbookLevels;
    for (int i = 0; i < count && n < max_levels; ++i) {
      if (sizes[i] > 0) {
        levels[n].price    = prices[i];
        levels[n].quantity = static_cast<double>(sizes[i]);
        ++n;
      }
    }
  }

  // ---- Single-level update (BidQuote / AskQuote / LimitOrderBook UPDATE) ----

  /// Insert, update, or delete one price level.
  /// qty > 0  → insert or update
  /// qty == 0 → delete
  void ApplyLevel(double price, double qty, bool is_ask) noexcept {
    auto& levels = is_ask ? asks_ : bids_;
    auto& n     = is_ask ? ask_count_ : bid_count_;
    uint32_t max_levels = sqc::kMaxOrderbookLevels;

    // Linear scan for matching price
    uint32_t pos = 0;
    while (pos < n && levels[pos].price != price) ++pos;

    if (pos < n) {
      // Found — update or delete
      if (qty > 0.0) {
        levels[pos].quantity = qty;
      } else {
        // Delete: shift remaining elements left
        for (uint32_t j = pos; j + 1 < n; ++j) {
          levels[j] = levels[j + 1];
        }
        --n;
      }
      return;
    }

    // Not found
    if (qty <= 0.0) return;           // nothing to delete
    if (n >= max_levels) return;      // book full

    // Find insertion position (maintain sorted order)
    // bids: descending  (levels[0] = highest price = best bid)
    // asks: ascending   (levels[0] = lowest price  = best ask)
    uint32_t insert_pos = 0;
    if (is_ask) {
      while (insert_pos < n && levels[insert_pos].price < price) ++insert_pos;
    } else {
      while (insert_pos < n && levels[insert_pos].price > price) ++insert_pos;
    }

    // Shift right and insert
    for (uint32_t j = n; j > insert_pos; --j) {
      levels[j] = levels[j - 1];
    }
    levels[insert_pos].price    = price;
    levels[insert_pos].quantity = qty;
    ++n;
  }

  // ---- Multi-part batch operations (UPDATE_TYPE_BEGIN/MIDDLE/END) ----

  bool batch_active() const noexcept { return batch_active_; }

  void BeginBatch() noexcept {
    pending_bid_count_ = 0;
    pending_ask_count_ = 0;
    batch_active_ = true;
  }

  void BufferLevel(double price, double qty, bool is_ask) noexcept {
    auto& buf = is_ask ? pending_asks_ : pending_bids_;
    auto& cnt = is_ask ? pending_ask_count_ : pending_bid_count_;
    if (cnt >= sqc::kMaxOrderbookLevels) return;
    buf[cnt].price    = price;
    buf[cnt].quantity = qty;
    ++cnt;
  }

  void CommitBatch() noexcept {
    for (uint32_t i = 0; i < pending_bid_count_; ++i) {
      ApplyLevel(pending_bids_[i].price, pending_bids_[i].quantity, false);
    }
    for (uint32_t i = 0; i < pending_ask_count_; ++i) {
      ApplyLevel(pending_asks_[i].price, pending_asks_[i].quantity, true);
    }
    pending_bid_count_ = 0;
    pending_ask_count_ = 0;
    batch_active_ = false;
  }

  void CancelBatch() noexcept {
    pending_bid_count_ = 0;
    pending_ask_count_ = 0;
    batch_active_ = false;
  }

  // ---- Clear ----

  void Clear() noexcept {
    bid_count_ = 0;
    ask_count_ = 0;
    pending_bid_count_ = 0;
    pending_ask_count_ = 0;
    batch_active_ = false;
  }

  // ---- Snapshot export ----

  /// Copy current book state into a DepthUpdateEvent for SHM publishing.
  void SnapshotTo(sqc::DepthUpdateEvent& out, uint64_t exchange_ts) const noexcept {
    out.last_update_id = 0;
    out.channel_id     = channel_id_;
    out.exchange_timestamp = exchange_ts;

    size_t n = 0;
    while (n < 11 && symbol_[n] != '\0') {
      out.symbol[n] = symbol_[n];
      ++n;
    }
    out.symbol[n] = '\0';

    out.bid_count = bid_count_;
    for (uint32_t i = 0; i < bid_count_; ++i) {
      out.bids[i] = bids_[i];
    }

    out.ask_count = ask_count_;
    for (uint32_t i = 0; i < ask_count_; ++i) {
      out.asks[i] = asks_[i];
    }
  }

  // ---- Accessors for testing ----

  uint32_t bid_count() const noexcept { return bid_count_; }
  uint32_t ask_count() const noexcept { return ask_count_; }
  const sqc::PriceLevel* bids() const noexcept { return bids_; }
  const sqc::PriceLevel* asks() const noexcept { return asks_; }

  uint32_t pending_bid_count() const noexcept { return pending_bid_count_; }
  uint32_t pending_ask_count() const noexcept { return pending_ask_count_; }

 private:
  // Main order book
  sqc::PriceLevel bids_[sqc::kMaxOrderbookLevels]{};
  sqc::PriceLevel asks_[sqc::kMaxOrderbookLevels]{};
  uint32_t bid_count_ = 0;
  uint32_t ask_count_ = 0;

  // Multi-part batch buffer
  sqc::PriceLevel pending_bids_[sqc::kMaxOrderbookLevels]{};
  sqc::PriceLevel pending_asks_[sqc::kMaxOrderbookLevels]{};
  uint32_t pending_bid_count_ = 0;
  uint32_t pending_ask_count_ = 0;
  bool batch_active_ = false;

  // Identity
  uint32_t channel_id_ = 0;
  char symbol_[12] = {};

  // Per-book spinlock
  std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};

}  // namespace rithmic
}  // namespace sqc
