#include <gtest/gtest.h>

#include "src/exchange/rithmic/rithmic_types.h"
#include "src/rithmic_gateway/rithmic_orderbook.h"

namespace sqc {
namespace rithmic {
namespace {

// ============================================================================
// Helpers
// ============================================================================

PriceLevel MakeLevel(double price, double qty) {
  PriceLevel l{};
  l.price = price;
  l.quantity = qty;
  return l;
}

void VerifySorted(const RithmicOrderBook& book) {
  const auto* bids = book.bids();
  for(uint32_t i = 1; i < book.bid_count(); ++i) {
    EXPECT_GT(bids[i - 1].price, bids[i].price) << "bids not descending at " << i;
  }
  const auto* asks = book.asks();
  for(uint32_t i = 1; i < book.ask_count(); ++i) {
    EXPECT_LT(asks[i - 1].price, asks[i].price) << "asks not ascending at " << i;
  }
}

// ============================================================================
// Init
// ============================================================================

TEST(RithmicOrderBookTest, InitSetsIdentity) {
  RithmicOrderBook book;
  book.Init(42, "ESZ4");
  EXPECT_EQ(book.channel_id(), 42u);
  EXPECT_STREQ(book.symbol(), "ESZ4");
  EXPECT_EQ(book.bid_count(), 0u);
  EXPECT_EQ(book.ask_count(), 0u);
}

// ============================================================================
// ApplyImage
// ============================================================================

TEST(RithmicOrderBookTest, ApplyImageBids) {
  RithmicOrderBook book;
  double prices[] = {4500.0, 4499.5, 4499.0};
  long long sizes[] = {10, 5, 20};
  book.ApplyImage(prices, sizes, 3, /*is_ask=*/false);

  EXPECT_EQ(book.bid_count(), 3u);
  EXPECT_EQ(book.ask_count(), 0u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 4500.0);
  EXPECT_DOUBLE_EQ(book.bids()[0].quantity, 10.0);
  EXPECT_DOUBLE_EQ(book.bids()[1].price, 4499.5);
  EXPECT_DOUBLE_EQ(book.bids()[1].quantity, 5.0);
  EXPECT_DOUBLE_EQ(book.bids()[2].price, 4499.0);
  EXPECT_DOUBLE_EQ(book.bids()[2].quantity, 20.0);
}

TEST(RithmicOrderBookTest, ApplyImageAsks) {
  RithmicOrderBook book;
  double prices[] = {4500.25, 4500.50, 4501.00};
  long long sizes[] = {15, 8, 3};
  book.ApplyImage(prices, sizes, 3, /*is_ask=*/true);

  EXPECT_EQ(book.ask_count(), 3u);
  EXPECT_EQ(book.bid_count(), 0u);
  EXPECT_DOUBLE_EQ(book.asks()[0].price, 4500.25);
  EXPECT_DOUBLE_EQ(book.asks()[1].price, 4500.50);
  EXPECT_DOUBLE_EQ(book.asks()[2].price, 4501.00);
}

TEST(RithmicOrderBookTest, ApplyImageSkipsZeroQuantity) {
  RithmicOrderBook book;
  double prices[] = {4500.0, 4499.0, 4498.0};
  long long sizes[] = {10, 0, 20};
  book.ApplyImage(prices, sizes, 3, /*is_ask=*/false);

  EXPECT_EQ(book.bid_count(), 2u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 4500.0);
  EXPECT_DOUBLE_EQ(book.bids()[1].price, 4498.0);
}

TEST(RithmicOrderBookTest, ApplyImageReplacesPrevious) {
  RithmicOrderBook book;
  double p1[] = {100.0, 99.0};
  long long s1[] = {5, 10};
  book.ApplyImage(p1, s1, 2, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 2u);

  double p2[] = {101.0};
  long long s2[] = {7};
  book.ApplyImage(p2, s2, 1, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 101.0);
}

// ============================================================================
// ApplyLevel — insert (sorted order)
// ============================================================================

TEST(RithmicOrderBookTest, ApplyLevelInsertEmpty) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.ApplyLevel(100.0, 10.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 100.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelInsertBestBid) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(101.0, 3.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 3u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 101.0);
  EXPECT_DOUBLE_EQ(book.bids()[1].price, 100.0);
  EXPECT_DOUBLE_EQ(book.bids()[2].price, 99.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelInsertMiddleBid) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 98.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(99.0, 7.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 3u);
  EXPECT_DOUBLE_EQ(book.bids()[1].price, 99.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelInsertTailBid) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(98.0, 3.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 3u);
  EXPECT_DOUBLE_EQ(book.bids()[2].price, 98.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelInsertBestAsk) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 101.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/true);
  book.ApplyLevel(99.0, 3.0, /*is_ask=*/true);
  EXPECT_EQ(book.ask_count(), 3u);
  EXPECT_DOUBLE_EQ(book.asks()[0].price, 99.0);
  VerifySorted(book);
}

// ============================================================================
// ApplyLevel — update / delete
// ============================================================================

TEST(RithmicOrderBookTest, ApplyLevelUpdateExisting) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(99.0, 15.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 2u);
  EXPECT_DOUBLE_EQ(book.bids()[1].quantity, 15.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelDeleteByZeroQty) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0, 98.0};
  long long s[] = {5, 10, 3};
  book.ApplyImage(p, s, 3, /*is_ask=*/false);
  book.ApplyLevel(99.0, 0.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 2u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 100.0);
  EXPECT_DOUBLE_EQ(book.bids()[1].price, 98.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelDeleteFirst) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(100.0, 0.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 99.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelDeleteLast) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyLevel(99.0, 0.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 100.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, ApplyLevelDeleteNonExistentNoOp) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0};
  long long s[] = {5};
  book.ApplyImage(p, s, 1, /*is_ask=*/false);
  book.ApplyLevel(999.0, 0.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
}

// ============================================================================
// Batch operations
// ============================================================================

TEST(RithmicOrderBookTest, BatchBeginMiddleEnd) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0, 98.0};
  long long s[] = {5, 10, 3};
  book.ApplyImage(p, s, 3, /*is_ask=*/false);

  book.BeginBatch();
  EXPECT_TRUE(book.batch_active());
  book.BufferLevel(99.0, 20.0, /*is_ask=*/false);
  book.BufferLevel(101.0, 7.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 3u);  // not yet applied
  EXPECT_EQ(book.pending_bid_count(), 2u);

  book.CommitBatch();
  EXPECT_FALSE(book.batch_active());
  EXPECT_EQ(book.bid_count(), 4u);
  EXPECT_DOUBLE_EQ(book.bids()[0].price, 101.0);
  EXPECT_DOUBLE_EQ(book.bids()[2].quantity, 20.0);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, BatchMixedBidAsk) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.BeginBatch();
  book.BufferLevel(100.0, 5.0, /*is_ask=*/false);
  book.BufferLevel(101.0, 3.0, /*is_ask=*/true);
  book.CommitBatch();
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_EQ(book.ask_count(), 1u);
  VerifySorted(book);
}

TEST(RithmicOrderBookTest, BatchCancel) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0};
  long long s[] = {5};
  book.ApplyImage(p, s, 1, /*is_ask=*/false);

  book.BeginBatch();
  book.BufferLevel(99.0, 10.0, /*is_ask=*/false);
  book.CancelBatch();
  EXPECT_FALSE(book.batch_active());
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_EQ(book.pending_bid_count(), 0u);
}

TEST(RithmicOrderBookTest, BatchAutoCancelOnNewBegin) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.BeginBatch();
  book.BufferLevel(99.0, 5.0, /*is_ask=*/false);
  book.BeginBatch();  // should reset
  EXPECT_EQ(book.pending_bid_count(), 0u);
}

// ============================================================================
// Clear
// ============================================================================

TEST(RithmicOrderBookTest, ClearEmptiesBook) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double p[] = {100.0, 99.0};
  long long s[] = {5, 10};
  book.ApplyImage(p, s, 2, /*is_ask=*/false);
  book.ApplyImage(p, s, 2, /*is_ask=*/true);
  book.Clear();
  EXPECT_EQ(book.bid_count(), 0u);
  EXPECT_EQ(book.ask_count(), 0u);
  EXPECT_FALSE(book.batch_active());
}

TEST(RithmicOrderBookTest, ClearDuringBatch) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.BeginBatch();
  book.BufferLevel(100.0, 5.0, /*is_ask=*/false);
  book.Clear();
  EXPECT_EQ(book.bid_count(), 0u);
  EXPECT_FALSE(book.batch_active());
}

// ============================================================================
// SnapshotTo
// ============================================================================

TEST(RithmicOrderBookTest, SnapshotTo) {
  RithmicOrderBook book;
  book.Init(7, "NQZ4");
  double bp[] = {15000.0, 14999.0};
  long long bs[] = {2, 4};
  double ap[] = {15001.0, 15002.0};
  long long as[] = {3, 5};
  book.ApplyImage(bp, bs, 2, /*is_ask=*/false);
  book.ApplyImage(ap, as, 2, /*is_ask=*/true);

  DepthUpdateEvent evt{};
  book.SnapshotTo(evt, 1234567890123456ULL);

  EXPECT_EQ(evt.channel_id, 7u);
  EXPECT_EQ(evt.exchange_timestamp, 1234567890123456ULL);
  EXPECT_STREQ(evt.symbol, "NQZ4");
  EXPECT_EQ(evt.bid_count, 2u);
  EXPECT_DOUBLE_EQ(evt.bids[0].price, 15000.0);
  EXPECT_DOUBLE_EQ(evt.bids[0].quantity, 2.0);
  EXPECT_EQ(evt.ask_count, 2u);
  EXPECT_DOUBLE_EQ(evt.asks[0].price, 15001.0);
}

TEST(RithmicOrderBookTest, SnapshotToEmptyBook) {
  RithmicOrderBook book;
  book.Init(0, "EMPTY");
  DepthUpdateEvent evt{};
  book.SnapshotTo(evt, 0);
  EXPECT_EQ(evt.bid_count, 0u);
  EXPECT_EQ(evt.ask_count, 0u);
  EXPECT_STREQ(evt.symbol, "EMPTY");
}

// ============================================================================
// Overflow
// ============================================================================

TEST(RithmicOrderBookTest, OverflowDropsExcess) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  for(int i = 0; i < static_cast<int>(kMaxOrderbookLevels) + 10; ++i) {
    book.ApplyLevel(100.0 + i * 0.25, 1.0, /*is_ask=*/true);
  }
  EXPECT_EQ(book.ask_count(), kMaxOrderbookLevels);
  EXPECT_DOUBLE_EQ(book.asks()[0].price, 100.0);
  VerifySorted(book);
}

// ============================================================================
// Sorted order with random inserts
// ============================================================================

TEST(RithmicOrderBookTest, RandomInsertMaintainsSortedOrder) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  double bid_p[] = {99.0, 101.0, 100.0, 98.0, 102.0};
  for(double p : bid_p) book.ApplyLevel(p, 1.0, /*is_ask=*/false);
  VerifySorted(book);
  const auto* bids = book.bids();
  EXPECT_DOUBLE_EQ(bids[0].price, 102.0);
  EXPECT_DOUBLE_EQ(bids[4].price, 98.0);

  double ask_p[] = {101.0, 99.0, 100.0, 102.0, 98.0};
  for(double p : ask_p) book.ApplyLevel(p, 1.0, /*is_ask=*/true);
  VerifySorted(book);
  EXPECT_DOUBLE_EQ(book.asks()[0].price, 98.0);
  EXPECT_DOUBLE_EQ(book.asks()[4].price, 102.0);
}

// ============================================================================
// Duplicate price handling
// ============================================================================

TEST(RithmicOrderBookTest, DuplicatePriceUpdatesQuantity) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.ApplyLevel(100.0, 5.0, /*is_ask=*/false);
  book.ApplyLevel(100.0, 10.0, /*is_ask=*/false);
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].quantity, 10.0);
}

TEST(RithmicOrderBookTest, BatchDuplicatePriceLastWins) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.BeginBatch();
  book.BufferLevel(100.0, 5.0, /*is_ask=*/false);
  book.BufferLevel(100.0, 15.0, /*is_ask=*/false);
  book.CommitBatch();
  EXPECT_EQ(book.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(book.bids()[0].quantity, 15.0);
}

// ============================================================================
// Multiple independent books
// ============================================================================

TEST(RithmicOrderBookTest, MultipleBooksIndependent) {
  RithmicOrderBook b1, b2;
  b1.Init(1, "ES");
  b2.Init(2, "NQ");
  b1.ApplyLevel(4500.0, 10.0, /*is_ask=*/false);
  b2.ApplyLevel(15000.0, 5.0, /*is_ask=*/false);
  EXPECT_EQ(b1.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(b1.bids()[0].price, 4500.0);
  EXPECT_EQ(b2.bid_count(), 1u);
  EXPECT_DOUBLE_EQ(b2.bids()[0].price, 15000.0);
}

// ============================================================================
// Spinlock
// ============================================================================

TEST(RithmicOrderBookTest, SpinlockSerializesAccess) {
  RithmicOrderBook book;
  book.Init(0, "TEST");
  book.Lock();
  book.ApplyLevel(100.0, 5.0, /*is_ask=*/false);
  book.Unlock();
  EXPECT_EQ(book.bid_count(), 1u);
}

// ============================================================================
// ResolveRithmicExchangeTimestamp
// ============================================================================

TEST(RithmicOrderBookTest, ResolveTimestampNonZeroUnchanged) {
  uint64_t out = 0;
  EXPECT_TRUE(ResolveRithmicExchangeTimestamp(1234567890123456ULL, 0, 0, out));
  EXPECT_EQ(out, 1234567890123456ULL);
}

TEST(RithmicOrderBookTest, ResolveTimestampZeroEmptyBookSkips) {
  uint64_t out = 999;
  EXPECT_FALSE(ResolveRithmicExchangeTimestamp(0, 0, 0, out));
}

TEST(RithmicOrderBookTest, ResolveTimestampZeroNonEmptyBookFallback) {
  uint64_t out = 0;
  EXPECT_TRUE(ResolveRithmicExchangeTimestamp(0, 1, 0, out));
  EXPECT_GT(out, 0u);
}

}  // namespace
}  // namespace rithmic
}  // namespace sqc
