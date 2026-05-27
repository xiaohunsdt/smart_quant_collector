#include <gtest/gtest.h>
#include <thread>
#include "src/exchange/shard_queue.h"

namespace sqc {
namespace {

TEST(ShardQueueTest, PushPopFIFO) {
  ShardQueue q(16);
  RawMessage m1; m1.data = "hello"; m1.size = 5;
  RawMessage m2; m2.data = "world"; m2.size = 5;
  q.Push(std::move(m1));
  q.Push(std::move(m2));
  RawMessage out;
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.data, "hello");
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.data, "world");
}

TEST(ShardQueueTest, EmptyQueueTryPopReturnsFalse) {
  ShardQueue q(16);
  RawMessage out;
  EXPECT_FALSE(q.TryPop(out));
}

TEST(ShardQueueTest, PoisonPill) {
  ShardQueue q(16);
  q.PushPoisonPill();
  RawMessage out = q.PopBlocking();
  EXPECT_TRUE(out.data.empty());
  EXPECT_EQ(out.size, 0u);
}

TEST(ShardQueueTest, ConcurrentPushPop) {
  ShardQueue q(256);
  constexpr int kCount = 1000;
  std::thread producer([&]() {
    for (int i = 0; i < kCount; ++i) {
      RawMessage m;
      m.data = std::to_string(i);
      q.Push(std::move(m));
    }
    q.PushPoisonPill();
  });
  int count = 0;
  while (true) {
    RawMessage out = q.PopBlocking();
    if (out.data.empty() && out.size == 0) break;
    EXPECT_EQ(out.data, std::to_string(count));
    count++;
  }
  producer.join();
  EXPECT_EQ(count, kCount);
}

}  // namespace
}  // namespace sqc
