#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/exchange/rithmic/rithmic_types.h"

namespace sqc {
namespace rithmic {
namespace {

using TestQueue = MpscRithmicQueue<64>;

// Helper: create a tick event
RithmicEvent MakeTick(uint32_t channel_id, double price, double quantity) {
  RithmicEvent e;
  e.type = EventType::TICK;
  e.channel_id = channel_id;
  e.tick.price = price;
  e.tick.quantity = quantity;
  e.tick.channel_id = channel_id;
  return e;
}

// Helper: create a depth event
RithmicEvent MakeDepth(uint32_t channel_id, uint32_t bids, uint32_t asks) {
  RithmicEvent e;
  e.type = EventType::DEPTH;
  e.channel_id = channel_id;
  e.depth.channel_id = channel_id;
  e.depth.bid_count = bids;
  e.depth.ask_count = asks;
  return e;
}

// ============================================================================
// SPSC correctness
// ============================================================================

TEST(RithmicQueueTest, PushPopSingle) {
  TestQueue q;
  ASSERT_TRUE(q.TryPush(MakeTick(1, 100.5, 10.0)));

  RithmicEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::TICK);
  EXPECT_EQ(out.channel_id, 1u);
  EXPECT_DOUBLE_EQ(out.tick.price, 100.5);
}

TEST(RithmicQueueTest, PushPopManyFifoOrder) {
  using LargeQueue = MpscRithmicQueue<128>;
  LargeQueue q;
  constexpr int kN = 100;

  for (int i = 0; i < kN; ++i)
    ASSERT_TRUE(q.TryPush(MakeTick(i, i * 1.0, i * 2.0)));

  for (int i = 0; i < kN; ++i) {
    RithmicEvent out;
    ASSERT_TRUE(q.TryPop(out));
    EXPECT_EQ(out.channel_id, static_cast<uint32_t>(i));
    EXPECT_DOUBLE_EQ(out.tick.price, i * 1.0);
  }
}

TEST(RithmicQueueTest, MixedEventTypesInOrder) {
  TestQueue q;

  ASSERT_TRUE(q.TryPush(MakeTick(1, 1.0, 1.0)));
  ASSERT_TRUE(q.TryPush(MakeDepth(2, 5, 10)));
  ASSERT_TRUE(q.TryPush(MakeTick(3, 3.0, 3.0)));

  RithmicEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::TICK);
  EXPECT_EQ(out.channel_id, 1u);

  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::DEPTH);
  EXPECT_EQ(out.channel_id, 2u);
  EXPECT_EQ(out.depth.bid_count, 5u);
  EXPECT_EQ(out.depth.ask_count, 10u);

  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::TICK);
  EXPECT_EQ(out.channel_id, 3u);
}

TEST(RithmicQueueTest, EmptyTryPopReturnsFalse) {
  TestQueue q;
  RithmicEvent out;
  EXPECT_FALSE(q.TryPop(out));
}

TEST(RithmicQueueTest, PopBlockingWaitsForData) {
  TestQueue q;
  std::atomic<bool> ready{false};

  std::thread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ready.store(true);
    (void)q.TryPush(MakeTick(42, 99.0, 5.0));
  });

  RithmicEvent out = q.PopBlocking();
  EXPECT_TRUE(ready.load());
  EXPECT_EQ(out.channel_id, 42u);
  producer.join();
}

// ============================================================================
// MPSC correctness
// ============================================================================

TEST(RithmicQueueTest, MpscMultiProducerNoLoss) {
  // Use a queue large enough that producers never block (1000 items).
  using LargeQueue = MpscRithmicQueue<2048>;
  LargeQueue q;
  constexpr int kProducers = 4;
  constexpr int kEach = 250;
  constexpr int kTotal = kProducers * kEach;

  auto produce = [&](int base) {
    for (int i = 0; i < kEach; ++i) {
      while (!q.TryPush(MakeTick(base * 1000 + i, base * 1.0, i * 1.0)))
        std::this_thread::yield();
    }
  };

  std::vector<std::thread> threads;
  for (int p = 0; p < kProducers; ++p) threads.emplace_back(produce, p);

  int got = 0;
  while (got < kTotal) {
    RithmicEvent out;
    if (q.TryPop(out)) {
      EXPECT_EQ(out.type, EventType::TICK);
      ++got;
    }
  }

  for (auto& t : threads) t.join();
  EXPECT_EQ(got, kTotal);
}

// ============================================================================
// Queue full behavior
// ============================================================================

TEST(RithmicQueueTest, TryPushFailsWhenFull) {
  TestQueue q;
  for (size_t i = 0; i < 64; ++i)
    ASSERT_TRUE(q.TryPush(MakeTick(i, 1.0, 1.0)));

  EXPECT_GE(q.size(), 60u);
  EXPECT_FALSE(q.TryPush(MakeTick(999, 1.0, 1.0)));
  EXPECT_GT(q.dropped_count(), 0u);
}

TEST(RithmicQueueTest, DropCountAccurate) {
  TestQueue q;
  for (size_t i = 0; i < 64; ++i)
    (void)q.TryPush(MakeTick(i, 1.0, 1.0));

  for (int i = 0; i < 10; ++i)
    (void)q.TryPush(MakeTick(i, 1.0, 1.0));

  EXPECT_GE(q.dropped_count(), 10u);

  int got = 0;
  RithmicEvent out;
  while (q.TryPop(out)) ++got;
  EXPECT_EQ(got, 64);
}

// ============================================================================
// Poison pill
// ============================================================================

TEST(RithmicQueueTest, PoisonPillIdentifiable) {
  TestQueue q;
  q.PushPoisonPill();

  RithmicEvent out = q.PopBlocking();
  EXPECT_EQ(out.type, EventType::NONE);
  EXPECT_EQ(out.channel_id, UINT32_MAX);
}

TEST(RithmicQueueTest, PoisonPillAfterNormalEvents) {
  TestQueue q;

  ASSERT_TRUE(q.TryPush(MakeTick(1, 1.0, 1.0)));
  ASSERT_TRUE(q.TryPush(MakeTick(2, 2.0, 2.0)));
  q.PushPoisonPill();

  RithmicEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::TICK);
  EXPECT_EQ(out.channel_id, 1u);

  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::TICK);
  EXPECT_EQ(out.channel_id, 2u);

  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::NONE);
  EXPECT_EQ(out.channel_id, UINT32_MAX);
}

// ============================================================================
// DepthUpdateEvent round-trip
// ============================================================================

TEST(RithmicQueueTest, DepthEventRoundTrip) {
  TestQueue q;

  RithmicEvent in = MakeDepth(7, 3, 5);
  in.depth.bids[0].price = 100.0;
  in.depth.bids[0].quantity = 10.0;
  in.depth.bids[1].price = 99.0;
  in.depth.bids[1].quantity = 20.0;
  in.depth.asks[0].price = 101.0;
  in.depth.asks[0].quantity = 15.0;
  in.depth.last_update_id = 12345;
  in.depth.exchange_timestamp = 67890;

  ASSERT_TRUE(q.TryPush(in));

  RithmicEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::DEPTH);
  EXPECT_EQ(out.channel_id, 7u);
  EXPECT_EQ(out.depth.bid_count, 3u);
  EXPECT_EQ(out.depth.ask_count, 5u);
  EXPECT_DOUBLE_EQ(out.depth.bids[0].price, 100.0);
  EXPECT_DOUBLE_EQ(out.depth.bids[0].quantity, 10.0);
  EXPECT_DOUBLE_EQ(out.depth.bids[1].price, 99.0);
  EXPECT_DOUBLE_EQ(out.depth.bids[1].quantity, 20.0);
  EXPECT_DOUBLE_EQ(out.depth.asks[0].price, 101.0);
  EXPECT_DOUBLE_EQ(out.depth.asks[0].quantity, 15.0);
  EXPECT_EQ(out.depth.last_update_id, 12345u);
  EXPECT_EQ(out.depth.exchange_timestamp, 67890u);
}

// ============================================================================
// BookTickerEvent round-trip
// ============================================================================

TEST(RithmicQueueTest, BookTickerEventRoundTrip) {
  TestQueue q;

  RithmicEvent in;
  in.type = EventType::BOOK_TICKER;
  in.channel_id = 3;
  in.book_ticker.channel_id = 3;
  in.book_ticker.best_bid_price = 99.5;
  in.book_ticker.best_bid_qty = 100.0;
  in.book_ticker.best_ask_price = 100.5;
  in.book_ticker.best_ask_qty = 200.0;

  ASSERT_TRUE(q.TryPush(in));

  RithmicEvent out;
  ASSERT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.type, EventType::BOOK_TICKER);
  EXPECT_EQ(out.channel_id, 3u);
  EXPECT_DOUBLE_EQ(out.book_ticker.best_bid_price, 99.5);
  EXPECT_DOUBLE_EQ(out.book_ticker.best_bid_qty, 100.0);
  EXPECT_DOUBLE_EQ(out.book_ticker.best_ask_price, 100.5);
  EXPECT_DOUBLE_EQ(out.book_ticker.best_ask_qty, 200.0);
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(RithmicQueueTest, InitiallyEmpty) {
  TestQueue q;
  EXPECT_EQ(q.size(), 0u);
  EXPECT_EQ(q.dropped_count(), 0u);
  EXPECT_EQ(q.capacity(), 64u);
}

TEST(RithmicQueueTest, SizeApproximateReflectsEvents) {
  TestQueue q;
  (void)q.TryPush(MakeTick(1, 1.0, 1.0));
  (void)q.TryPush(MakeTick(2, 2.0, 2.0));
  EXPECT_GE(q.size(), 2u);

  RithmicEvent out;
  (void)q.TryPop(out);
  (void)q.TryPop(out);
  EXPECT_LE(q.size(), 2u);
}

}  // namespace
}  // namespace rithmic
}  // namespace sqc
