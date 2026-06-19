#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "storage/channel_registry.h"

namespace sqc {
namespace {

TEST(ChannelRegistryTest, RegisterAndFind) {
  StorageChannelRegistry r;
  r.Register(0, "binance", "perpetual", "BTCUSDT", 10);
  r.Register(1, "gateio", "spot", "ETH_USDT", 5);

  const ChannelMeta* m0 = r.Find(0);
  ASSERT_NE(m0, nullptr);
  EXPECT_EQ(m0->exchange, "binance");
  EXPECT_EQ(m0->market_type, "perpetual");
  EXPECT_EQ(m0->symbol, "BTCUSDT");
  EXPECT_EQ(m0->depth_level, 10u);

  const ChannelMeta* m1 = r.Find(1);
  ASSERT_NE(m1, nullptr);
  EXPECT_EQ(m1->symbol, "ETH_USDT");
  EXPECT_EQ(m1->depth_level, 5u);
}

TEST(ChannelRegistryTest, FindUnknownReturnsNull) {
  StorageChannelRegistry r;
  r.Register(0, "binance", "spot", "BTCUSDT", 5);
  EXPECT_EQ(r.Find(999), nullptr);
}

TEST(ChannelRegistryTest, SizeTracksRegistrations) {
  StorageChannelRegistry r;
  EXPECT_EQ(r.Size(), 0u);
  r.Register(0, "binance", "spot", "BTCUSDT", 5);
  r.Register(1, "binance", "spot", "ETHUSDT", 5);
  EXPECT_EQ(r.Size(), 2u);
}

TEST(ChannelRegistryTest, FreezeMarksFrozen) {
  StorageChannelRegistry r;
  EXPECT_FALSE(r.IsFrozen());
  r.Freeze();
  EXPECT_TRUE(r.IsFrozen());
}

// The concurrency fix this registry exists to enforce: once frozen, a late
// Register() must NOT mutate the map (the old DolphinDBClient only asserted
// this in Debug, silently corrupting the map under concurrent readers in
// Release). Here we verify the write is rejected and the map is untouched.
TEST(ChannelRegistryTest, LateRegisterAfterFreezeIsRejected) {
  StorageChannelRegistry r;
  r.Register(0, "binance", "spot", "BTCUSDT", 5);
  r.Freeze();

  // This late registration must be a no-op.
  r.Register(1, "binance", "spot", "ETHUSDT", 5);
  EXPECT_EQ(r.Size(), 1u);
  EXPECT_EQ(r.Find(1), nullptr);
  // The originally-registered channel is still resolvable.
  ASSERT_NE(r.Find(0), nullptr);
}

// Concurrent reads after freeze must be safe (the documented hot-path
// contract). A light stress: many threads resolve channels while no mutation
// is possible.
TEST(ChannelRegistryTest, ConcurrentReadsAfterFreeze) {
  StorageChannelRegistry r;
  for (uint32_t i = 0; i < 100; ++i) {
    r.Register(i, "binance", "spot", "SYM", i);
  }
  r.Freeze();

  std::vector<std::thread> threads;
  std::atomic<int> hits{0};
  for (int t = 0; t < 8; ++t) {
    threads.emplace_back([&]() {
      for (uint32_t i = 0; i < 100; ++i) {
        if (r.Find(i) != nullptr) hits.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }
  for (auto& th : threads) th.join();
  EXPECT_EQ(hits.load(), 8 * 100);
}

}  // namespace
}  // namespace sqc
