#include <gtest/gtest.h>
#include <thread>
#include <cstring>
#include "src/exchange/shard_queue.h"

namespace sqc {
namespace {

TEST(ShardQueueTest, PushPopFIFO) {
  ShardQueue q(16);
  auto d1 = std::shared_ptr<char[]>(new char[6]); std::memcpy(d1.get(), "hello", 5);
  auto d2 = std::shared_ptr<char[]>(new char[6]); std::memcpy(d2.get(), "world", 5);
  RawMessage m1; m1.data = d1; m1.size = 5;
  RawMessage m2; m2.data = d2; m2.size = 5;
  q.Push(std::move(m1)); q.Push(std::move(m2));
  RawMessage out;
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.size, 5u);
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.size, 5u);
}

TEST(ShardQueueTest, PoisonPill) {
  ShardQueue q(16);
  q.PushPoisonPill();
  RawMessage out = q.PopBlocking();
  EXPECT_EQ(out.data, nullptr);
  EXPECT_EQ(out.size, 0u);
}

TEST(ShardQueueTest, ConcurrentPushPop) {
  ShardQueue q(256);
  constexpr int kCount = 100;
  std::thread producer([&]() {
    for (int i = 0; i < kCount; ++i) {
      RawMessage m;
      m.data = std::shared_ptr<char[]>(new char[1]);
      m.size = 1;
      q.Push(std::move(m));
    }
    q.PushPoisonPill();
  });
  int count = 0;
  while (true) {
    RawMessage out = q.PopBlocking();
    if (!out.data) break;
    count++;
  }
  producer.join();
  EXPECT_EQ(count, kCount);
}

}  // namespace
}  // namespace sqc
