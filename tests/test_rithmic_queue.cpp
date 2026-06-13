#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "src/common/tick_data.h"
#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {
namespace {

using TickQueue = MpscRithmicQueue<TickData, 64>;
using DepthQueue = MpscRithmicQueue<DepthUpdateEvent, 64>;
using BookTickQueue = MpscRithmicQueue<BookTickerEvent, 64>;

// Helper: create a tick
TickData MakeTick(uint32_t channel_id, double price, double quantity) {
  TickData t{};
  t.channel_id = channel_id;
  t.price = price;
  t.quantity = quantity;
  return t;
}

// Helper: create a depth event
DepthUpdateEvent MakeDepth(uint32_t channel_id, uint32_t bids, uint32_t asks) {
  DepthUpdateEvent d{};
  d.channel_id = channel_id;
  d.bid_count = bids;
  d.ask_count = asks;
  return d;
}

// Helper: create a book ticker event
BookTickerEvent MakeBookTicker(uint32_t channel_id, double bid_p, double bid_q, double ask_p, double ask_q) {
  BookTickerEvent bt{};
  bt.channel_id = channel_id;
  bt.best_bid_price = bid_p;
  bt.best_bid_qty = bid_q;
  bt.best_ask_price = ask_p;
  bt.best_ask_qty = ask_q;
  return bt;
}

// ============================================================================
// Tick queue: SPSC correctness
// ============================================================================

TEST(RithmicQueueTest, TickPushPopSingle) {
  TickQueue q;
  ASSERT_TRUE(q.TryPush(MakeTick(1, 100.5, 10.0)));

  TickData out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.channel_id, 1u);
  EXPECT_DOUBLE_EQ(out.price, 100.5);
}

TEST(RithmicQueueTest, TickPushPopManyFifoOrder) {
  using LargeTickQueue = MpscRithmicQueue<TickData, 128>;
  LargeTickQueue q;
  constexpr int kN = 100;

  for(int i = 0; i < kN; ++i) ASSERT_TRUE(q.TryPush(MakeTick(i, i * 1.0, i * 2.0)));

  for(int i = 0; i < kN; ++i) {
    TickData out;
    ASSERT_TRUE(q.TryPop(out));
    EXPECT_EQ(out.channel_id, static_cast<uint32_t>(i));
    EXPECT_DOUBLE_EQ(out.price, i * 1.0);
  }
}

TEST(RithmicQueueTest, TickEmptyTryPopReturnsFalse) {
  TickQueue q;
  TickData out;
  EXPECT_FALSE(q.TryPop(out));
}

TEST(RithmicQueueTest, TickPopBlockingWaitsForData) {
  TickQueue q;
  std::atomic<bool> ready{false};

  std::thread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ready.store(true);
    (void)q.TryPush(MakeTick(42, 99.0, 5.0));
  });

  TickData out = q.PopBlocking();
  EXPECT_TRUE(ready.load());
  EXPECT_EQ(out.channel_id, 42u);
  producer.join();
}

// ============================================================================
// MPSC correctness (Tick queue, multiple producers)
// ============================================================================

TEST(RithmicQueueTest, MpscMultiProducerNoLoss) {
  using LargeTickQueue = MpscRithmicQueue<TickData, 2048>;
  LargeTickQueue q;
  constexpr int kProducers = 4;
  constexpr int kEach = 250;
  constexpr int kTotal = kProducers * kEach;

  auto produce = [&](int base) {
    for(int i = 0; i < kEach; ++i) {
      while(!q.TryPush(MakeTick(base * 1000 + i, base * 1.0, i * 1.0))) std::this_thread::yield();
    }
  };

  std::vector<std::thread> threads;
  for(int p = 0; p < kProducers; ++p) threads.emplace_back(produce, p);

  int got = 0;
  while(got < kTotal) {
    TickData out;
    if(q.TryPop(out)) {
      ++got;
    }
  }

  for(auto& t : threads) t.join();
  EXPECT_EQ(got, kTotal);
}

// ============================================================================
// Queue full behavior (Tick queue)
// ============================================================================

TEST(RithmicQueueTest, TickTryPushFailsWhenFull) {
  TickQueue q;
  for(size_t i = 0; i < 64; ++i) ASSERT_TRUE(q.TryPush(MakeTick(i, 1.0, 1.0)));

  EXPECT_GE(q.size(), 60u);
  EXPECT_FALSE(q.TryPush(MakeTick(999, 1.0, 1.0)));
  EXPECT_GT(q.dropped_count(), 0u);
}

TEST(RithmicQueueTest, TickDropCountAccurate) {
  TickQueue q;
  for(size_t i = 0; i < 64; ++i) (void)q.TryPush(MakeTick(i, 1.0, 1.0));

  for(int i = 0; i < 10; ++i) (void)q.TryPush(MakeTick(i, 1.0, 1.0));

  EXPECT_GE(q.dropped_count(), 10u);

  int got = 0;
  TickData out;
  while(q.TryPop(out)) ++got;
  EXPECT_EQ(got, 64);
}

// ============================================================================
// DepthUpdateEvent round-trip
// ============================================================================

TEST(RithmicQueueTest, DepthEventRoundTrip) {
  DepthQueue q;

  DepthUpdateEvent in = MakeDepth(7, 3, 5);
  in.bids[0].price = 100.0;
  in.bids[0].quantity = 10.0;
  in.bids[1].price = 99.0;
  in.bids[1].quantity = 20.0;
  in.asks[0].price = 101.0;
  in.asks[0].quantity = 15.0;
  in.last_update_id = 12345;
  in.exchange_timestamp = 67890;

  ASSERT_TRUE(q.TryPush(in));

  DepthUpdateEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.channel_id, 7u);
  EXPECT_EQ(out.bid_count, 3u);
  EXPECT_EQ(out.ask_count, 5u);
  EXPECT_DOUBLE_EQ(out.bids[0].price, 100.0);
  EXPECT_DOUBLE_EQ(out.bids[0].quantity, 10.0);
  EXPECT_DOUBLE_EQ(out.bids[1].price, 99.0);
  EXPECT_DOUBLE_EQ(out.bids[1].quantity, 20.0);
  EXPECT_DOUBLE_EQ(out.asks[0].price, 101.0);
  EXPECT_DOUBLE_EQ(out.asks[0].quantity, 15.0);
  EXPECT_EQ(out.last_update_id, 12345u);
  EXPECT_EQ(out.exchange_timestamp, 67890u);
}

// ============================================================================
// BookTickerEvent round-trip
// ============================================================================

TEST(RithmicQueueTest, BookTickerEventRoundTrip) {
  BookTickQueue q;

  BookTickerEvent in{};
  in.channel_id = 3;
  in.best_bid_price = 99.5;
  in.best_bid_qty = 100.0;
  in.best_ask_price = 100.5;
  in.best_ask_qty = 200.0;

  ASSERT_TRUE(q.TryPush(in));

  BookTickerEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.channel_id, 3u);
  EXPECT_DOUBLE_EQ(out.best_bid_price, 99.5);
  EXPECT_DOUBLE_EQ(out.best_bid_qty, 100.0);
  EXPECT_DOUBLE_EQ(out.best_ask_price, 100.5);
  EXPECT_DOUBLE_EQ(out.best_ask_qty, 200.0);
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(RithmicQueueTest, TickInitiallyEmpty) {
  TickQueue q;
  EXPECT_EQ(q.size(), 0u);
  EXPECT_EQ(q.dropped_count(), 0u);
  EXPECT_EQ(q.capacity(), 64u);
}

TEST(RithmicQueueTest, TickSizeApproximateReflectsEvents) {
  TickQueue q;
  (void)q.TryPush(MakeTick(1, 1.0, 1.0));
  (void)q.TryPush(MakeTick(2, 2.0, 2.0));
  EXPECT_GE(q.size(), 2u);

  TickData out;
  (void)q.TryPop(out);
  (void)q.TryPop(out);
  EXPECT_LE(q.size(), 2u);
}

}  // namespace
}  // namespace rithmic
}  // namespace sqc
