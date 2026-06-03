#include <gtest/gtest.h>

#include <cstring>
#include <thread>

#include "src/exchange/shard_queue.h"

namespace sqc {
namespace {

TEST(ShardQueueTest, PushPopFIFO) {
  ShardQueue q(16);
  RawMessage m1;
  (void)m1.allocate(5);
  std::memcpy(m1.buffer(), "hello", 5);
  m1.size = 5;
  RawMessage m2;
  (void)m2.allocate(5);
  std::memcpy(m2.buffer(), "world", 5);
  m2.size = 5;
  q.Push(std::move(m1));
  q.Push(std::move(m2));
  RawMessage out;
  EXPECT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.size, 5u);
  EXPECT_TRUE(q.TryPop(out));
  EXPECT_EQ(out.size, 5u);
}

TEST(ShardQueueTest, PoisonPill) {
  ShardQueue q(16);
  q.PushPoisonPill();
  RawMessage out = q.PopBlocking();
  EXPECT_EQ(out.size, 0u);  // poison pill: size == 0
}

TEST(ShardQueueTest, ConcurrentPushPop) {
  ShardQueue q(256);
  constexpr int kCount = 100;
  std::thread producer([&]() {
    for(int i = 0; i < kCount; ++i) {
      RawMessage m;
      (void)m.allocate(1);
      m.size = 1;
      q.Push(std::move(m));
    }
    q.PushPoisonPill();
  });
  int count = 0;
  while(true) {
    RawMessage out = q.PopBlocking();
    if(out.size == 0) break;  // poison pill
    count++;
  }
  producer.join();
  EXPECT_EQ(count, kCount);
}

}  // namespace
}  // namespace sqc
