#include <gtest/gtest.h>

#include "src/orderbook/local_lob.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace {

TEST(LocalLOBTest, ApplySnapshot) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 100;
  snap.bids[0] = {50000.0, 1.5};
  snap.bids[1] = {49900.0, 2.0};
  snap.bid_count = 2;
  snap.asks[0] = {50100.0, 3.0};
  snap.asks[1] = {50200.0, 4.0};
  snap.ask_count = 2;

  lob.ApplySnapshot(snap);

  EXPECT_EQ(lob.last_update_id(), 100);
  EXPECT_DOUBLE_EQ(lob.BestBid(), 50000.0);
  EXPECT_DOUBLE_EQ(lob.BestAsk(), 50100.0);
  PriceLevel top_bids[10], top_asks[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 2u);
  EXPECT_EQ(lob.TopAsks(top_asks, 10), 2u);
}

TEST(LocalLOBTest, UpdateDepthUpsert) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {50000.0, 1.0};
  snap.bid_count = 1;
  snap.asks[0] = {50100.0, 2.0};
  snap.ask_count = 1;
  lob.ApplySnapshot(snap);

  // Update existing level
  DepthUpdateEvent event{};
  event.first_update_id = 1;
  event.last_update_id = 2;
  event.bids[0] = {50000.0, 3.0};  // upsert quantity
  event.bid_count = 1;
  lob.UpdateDepth(event);

  EXPECT_DOUBLE_EQ(lob.BestBid(), 50000.0);
  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 1u);
  EXPECT_DOUBLE_EQ(top_bids[0].quantity, 3.0);
}

TEST(LocalLOBTest, UpdateDepthDelete) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {50000.0, 1.0};
  snap.bids[1] = {49900.0, 2.0};
  snap.bid_count = 2;
  lob.ApplySnapshot(snap);

  // Delete level with qty=0
  DepthUpdateEvent event{};
  event.first_update_id = 1;
  event.last_update_id = 2;
  event.bids[0] = {50000.0, 0.0};  // delete
  event.bid_count = 1;
  lob.UpdateDepth(event);

  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 1u);
  EXPECT_DOUBLE_EQ(lob.BestBid(), 49900.0);  // next best
}

TEST(LocalLOBTest, ForceAlignWithEvent) {
  LocalLOB lob;
  // Build up some state
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 100;
  snap.bids[0] = {50000.0, 1.0};
  snap.bid_count = 1;
  lob.ApplySnapshot(snap);

  // Force-align with new event (clears old state)
  DepthUpdateEvent event{};
  event.first_update_id = 200;
  event.last_update_id = 200;
  event.bids[0] = {60000.0, 5.0};
  event.bid_count = 1;
  lob.ForceAlignWithEvent(event);

  EXPECT_EQ(lob.last_update_id(), 200);
  EXPECT_DOUBLE_EQ(lob.BestBid(), 60000.0);
  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 1u);
}

TEST(LocalLOBTest, UpdateBestPriceBidMovesDownMaintainsDescendingOrder) {
  LocalLOB lob;
  // Set up: bids = [100, 99, 98]
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {100.0, 1.0};
  snap.bids[1] = {99.0, 2.0};
  snap.bids[2] = {98.0, 3.0};
  snap.bid_count = 3;
  lob.ApplySnapshot(snap);

  // Best bid moves DOWN from 100 to 97 (worse price)
  bool changed = lob.UpdateBestPrice(97.0, 5.0, 0.0, 0.0);
  EXPECT_TRUE(changed);

  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 3u);
  // Should be sorted descending: 99, 98, 97
  EXPECT_DOUBLE_EQ(top_bids[0].price, 99.0);
  EXPECT_DOUBLE_EQ(top_bids[0].quantity, 2.0);
  EXPECT_DOUBLE_EQ(top_bids[1].price, 98.0);
  EXPECT_DOUBLE_EQ(top_bids[1].quantity, 3.0);
  EXPECT_DOUBLE_EQ(top_bids[2].price, 97.0);
  EXPECT_DOUBLE_EQ(top_bids[2].quantity, 5.0);
  EXPECT_DOUBLE_EQ(lob.BestBid(), 99.0);
}

TEST(LocalLOBTest, UpdateBestPriceBidMovesUpMaintainsDescendingOrder) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {100.0, 1.0};
  snap.bids[1] = {99.0, 2.0};
  snap.bid_count = 2;
  lob.ApplySnapshot(snap);

  // Best bid moves UP from 100 to 102 (better price)
  bool changed = lob.UpdateBestPrice(102.0, 8.0, 0.0, 0.0);
  EXPECT_TRUE(changed);

  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 2u);
  EXPECT_DOUBLE_EQ(top_bids[0].price, 102.0);
  EXPECT_DOUBLE_EQ(top_bids[1].price, 99.0);
}

TEST(LocalLOBTest, UpdateBestPriceAskMovesUpMaintainsAscendingOrder) {
  LocalLOB lob;
  // asks = [100, 101, 102]
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.asks[0] = {100.0, 1.0};
  snap.asks[1] = {101.0, 2.0};
  snap.asks[2] = {102.0, 3.0};
  snap.ask_count = 3;
  lob.ApplySnapshot(snap);

  // Best ask moves UP from 100 to 103 (worse price)
  bool changed = lob.UpdateBestPrice(0.0, 0.0, 103.0, 5.0);
  EXPECT_TRUE(changed);

  PriceLevel top_asks[10];
  EXPECT_EQ(lob.TopAsks(top_asks, 10), 3u);
  // Should be sorted ascending: 101, 102, 103
  EXPECT_DOUBLE_EQ(top_asks[0].price, 101.0);
  EXPECT_DOUBLE_EQ(top_asks[0].quantity, 2.0);
  EXPECT_DOUBLE_EQ(top_asks[1].price, 102.0);
  EXPECT_DOUBLE_EQ(top_asks[1].quantity, 3.0);
  EXPECT_DOUBLE_EQ(top_asks[2].price, 103.0);
  EXPECT_DOUBLE_EQ(top_asks[2].quantity, 5.0);
  EXPECT_DOUBLE_EQ(lob.BestAsk(), 101.0);
}

TEST(LocalLOBTest, UpdateBestPriceAskMovesDownMaintainsAscendingOrder) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.asks[0] = {100.0, 1.0};
  snap.asks[1] = {101.0, 2.0};
  snap.ask_count = 2;
  lob.ApplySnapshot(snap);

  // Best ask moves DOWN from 100 to 98 (better price)
  bool changed = lob.UpdateBestPrice(0.0, 0.0, 98.0, 8.0);
  EXPECT_TRUE(changed);

  PriceLevel top_asks[10];
  EXPECT_EQ(lob.TopAsks(top_asks, 10), 2u);
  EXPECT_DOUBLE_EQ(top_asks[0].price, 98.0);
  EXPECT_DOUBLE_EQ(top_asks[1].price, 101.0);
}

TEST(LocalLOBTest, UpdateBestPriceDeduplicatesExistingBidLevel) {
  LocalLOB lob;
  // bids = [100, 99, 98]
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {100.0, 1.0};
  snap.bids[1] = {99.0, 2.0};
  snap.bids[2] = {98.0, 3.0};
  snap.bid_count = 3;
  lob.ApplySnapshot(snap);

  // Best bid becomes 99, which already exists at index 1
  bool changed = lob.UpdateBestPrice(99.0, 7.0, 0.0, 0.0);
  EXPECT_TRUE(changed);

  PriceLevel top_bids[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 2u);
  // 100 was overwritten, 99 was deduplicated → [99, 98]
  EXPECT_DOUBLE_EQ(top_bids[0].price, 99.0);
  EXPECT_DOUBLE_EQ(top_bids[0].quantity, 7.0);
  EXPECT_DOUBLE_EQ(top_bids[1].price, 98.0);
  EXPECT_DOUBLE_EQ(top_bids[1].quantity, 3.0);
}

TEST(LocalLOBTest, UpdateBestPriceDeduplicatesExistingAskLevel) {
  LocalLOB lob;
  // asks = [100, 101, 102]
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.asks[0] = {100.0, 1.0};
  snap.asks[1] = {101.0, 2.0};
  snap.asks[2] = {102.0, 3.0};
  snap.ask_count = 3;
  lob.ApplySnapshot(snap);

  // Best ask becomes 101, which already exists at index 1
  bool changed = lob.UpdateBestPrice(0.0, 0.0, 101.0, 7.0);
  EXPECT_TRUE(changed);

  PriceLevel top_asks[10];
  EXPECT_EQ(lob.TopAsks(top_asks, 10), 2u);
  EXPECT_DOUBLE_EQ(top_asks[0].price, 101.0);
  EXPECT_DOUBLE_EQ(top_asks[0].quantity, 7.0);
  EXPECT_DOUBLE_EQ(top_asks[1].price, 102.0);
}

TEST(LocalLOBTest, UpdateBestPriceNoChangeReturnsFalse) {
  LocalLOB lob;
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  snap.bids[0] = {100.0, 1.0};
  snap.bids[1] = {99.0, 2.0};
  snap.bid_count = 2;
  snap.asks[0] = {101.0, 3.0};
  snap.asks[1] = {102.0, 4.0};
  snap.ask_count = 2;
  lob.ApplySnapshot(snap);

  // Same best prices — should return false
  bool changed = lob.UpdateBestPrice(100.0, 1.0, 101.0, 3.0);
  EXPECT_FALSE(changed);
}

TEST(LocalLOBTest, UpdateBestPriceFromEmptyInitializesLevels) {
  LocalLOB lob;

  bool changed = lob.UpdateBestPrice(100.0, 5.0, 101.0, 6.0);
  EXPECT_TRUE(changed);
  EXPECT_DOUBLE_EQ(lob.BestBid(), 100.0);
  EXPECT_DOUBLE_EQ(lob.BestAsk(), 101.0);
  EXPECT_DOUBLE_EQ(lob.BestBidVolume(), 5.0);
  EXPECT_DOUBLE_EQ(lob.BestAskVolume(), 6.0);

  PriceLevel top_bids[10], top_asks[10];
  EXPECT_EQ(lob.TopBids(top_bids, 10), 1u);
  EXPECT_EQ(lob.TopAsks(top_asks, 10), 1u);
}

}  // namespace
}  // namespace sqc
