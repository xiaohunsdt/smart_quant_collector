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

}  // namespace
}  // namespace sqc
