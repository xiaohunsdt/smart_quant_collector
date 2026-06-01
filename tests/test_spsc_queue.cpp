#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include "src/common/spsc_queue.h"

namespace sqc {
namespace {

// The monotonic-counter SPSC algorithm uses ALL kCapacity slots (unlike the
// legacy wrapping algorithm which reserved one slot). kCapacity=4 → 4 usable.

TEST(SPSCQueueTest, BasicPushPop) {
  SPSCQueue<int, 4> q;
  int val;

  EXPECT_TRUE(q.try_push(1));
  EXPECT_TRUE(q.try_push(2));
  EXPECT_TRUE(q.try_push(3));
  EXPECT_TRUE(q.try_push(4));
  EXPECT_FALSE(q.try_push(99));  // full (all 4 slots used)

  EXPECT_TRUE(q.try_pop(val)); EXPECT_EQ(val, 1);
  EXPECT_TRUE(q.try_pop(val)); EXPECT_EQ(val, 2);
  EXPECT_TRUE(q.try_pop(val)); EXPECT_EQ(val, 3);
  EXPECT_TRUE(q.try_pop(val)); EXPECT_EQ(val, 4);
  EXPECT_FALSE(q.try_pop(val));  // empty
}

TEST(SPSCQueueTest, WrapAroundPreservesData) {
  SPSCQueue<int, 4> q;
  int val;

  for (int cycle = 0; cycle < 5; ++cycle) {
    for (int i = 0; i < 4; ++i)
      ASSERT_TRUE(q.try_push(cycle * 100 + i));
    ASSERT_FALSE(q.try_push(-1));  // full

    for (int i = 0; i < 4; ++i) {
      ASSERT_TRUE(q.try_pop(val));
      EXPECT_EQ(val, cycle * 100 + i);
    }
    ASSERT_FALSE(q.try_pop(val));  // empty
  }
}

TEST(SPSCQueueTest, FullDetectionAfterWrap) {
  SPSCQueue<int, 4> q;
  int val;

  // Advance counters well past kCapacity via multiple fill/drain cycles.
  for (int cycle = 0; cycle < 3; ++cycle) {
    for (int i = 0; i < 4; ++i) EXPECT_TRUE(q.try_push(i));
    for (int i = 0; i < 4; ++i) EXPECT_TRUE(q.try_pop(val));
  }

  // Queue is empty. Fill again — must detect full after 4 pushes.
  for (int i = 0; i < 4; ++i) EXPECT_TRUE(q.try_push(i + 10));
  EXPECT_FALSE(q.try_push(99));
}

TEST(SPSCQueueTest, TriviallyCopyableType) {
  struct Point { double x, y; };
  static_assert(std::is_trivially_copyable_v<Point>);

  SPSCQueue<Point, 8> q;
  EXPECT_TRUE(q.try_push({1.0, 2.0}));
  Point out{};
  EXPECT_TRUE(q.try_pop(out));
  EXPECT_DOUBLE_EQ(out.x, 1.0);
  EXPECT_DOUBLE_EQ(out.y, 2.0);
}

TEST(SPSCQueueTest, SingleProducerSingleConsumer) {
  constexpr size_t kCount = 100000;
  SPSCQueue<uint64_t, 1024> q;
  std::atomic<bool> producer_done{false};

  std::thread producer([&]() {
    for (size_t i = 0; i < kCount; ++i)
      while (!q.try_push(i)) { /* spin */ }
    producer_done.store(true, std::memory_order_release);
  });

  std::thread consumer([&]() {
    uint64_t expected = 0;
    uint64_t val;
    for (size_t i = 0; i < kCount; ++i) {
      while (!q.try_pop(val)) { /* spin */ }
      EXPECT_EQ(val, expected++);
    }
  });

  producer.join();
  consumer.join();
  EXPECT_TRUE(producer_done.load(std::memory_order_acquire));

  uint64_t dummy;
  EXPECT_FALSE(q.try_pop(dummy));
}

}  // namespace
}  // namespace sqc
