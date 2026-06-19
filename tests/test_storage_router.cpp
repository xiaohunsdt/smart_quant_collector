#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/storage_router.h"

namespace sqc {
namespace {

TickData MakeTestTick(uint32_t channel_id, uint64_t ts, const char* symbol, double price, double qty) {
  TickData t{};
  t.channel_id = channel_id;
  t.exchange_timestamp = ts;
  t.price = price;
  t.quantity = qty;
  t.trade_id = 1;
  t.local_diff = 100;
  std::strncpy(t.symbol, symbol, sizeof(t.symbol) - 1);
  t.is_buyer_maker = false;
  return t;
}

ChannelInfo MakeChannelInfo(uint32_t id, const std::string& ex, ChannelType type, const std::string& sym) {
  ChannelInfo info;
  info.id = id;
  info.exchange = ex;
  info.type = type;
  info.symbol = sym;
  info.depth_level = 5;
  return info;
}

class StorageRouterTest : public ::testing::Test {
 protected:
  void TearDown() override { StorageRouter::ResetForTesting(); }
};

TEST_F(StorageRouterTest, RegisterChannelNoCrash) {
  auto& router = StorageRouter::Instance();
  auto info = MakeChannelInfo(1, "test", ChannelType::Spot, "BTCUSDT");
  // persist_to_disk=false → no-op
  router.RegisterChannel(1, info, false);
  router.FlushAndClose();
}

TEST_F(StorageRouterTest, RouteTickDifferentChannels) {
  auto& router = StorageRouter::Instance();

  auto info1 = MakeChannelInfo(100, "test", ChannelType::Spot, "SYM1");
  auto info2 = MakeChannelInfo(200, "test", ChannelType::Spot, "SYM2");
  router.RegisterChannel(100, info1, true);
  router.RegisterChannel(200, info2, true);

  router.RouteTick(MakeTestTick(100, 1000, "SYM1", 10.0, 1.0), info1);
  router.RouteTick(MakeTestTick(200, 2000, "SYM2", 20.0, 2.0), info2);

  // Unregistered channel_id — should be a no-op.
  auto info3 = MakeChannelInfo(999, "test", ChannelType::Spot, "UNKNOWN");
  router.RouteTick(MakeTestTick(999, 3000, "UNKNOWN", 30.0, 3.0), info3);

  router.FlushAndClose();
}

TEST_F(StorageRouterTest, ConcurrentRouteTick) {
  auto& router = StorageRouter::Instance();
  constexpr uint32_t kChannels = 4;
  constexpr size_t kPerThread = 1000;

  for(uint32_t i = 0; i < kChannels; ++i) {
    auto info = MakeChannelInfo(i, "test", ChannelType::Spot, std::string("SYM") + std::to_string(i));
    router.RegisterChannel(i, info, true);
  }

  std::vector<std::thread> threads;
  for(uint32_t t = 0; t < kChannels; ++t) {
    threads.emplace_back([&router, t]() {
      auto info = MakeChannelInfo(t, "test", ChannelType::Spot, std::string("SYM") + std::to_string(t));
      for(size_t j = 0; j < kPerThread; ++j) {
        router.RouteTick(MakeTestTick(t, j, info.symbol.c_str(), 100.0 + j, 1.0), info);
      }
    });
  }
  for(auto& th : threads) th.join();
  router.FlushAndClose();
}

TEST_F(StorageRouterTest, RouteOrderbookUsesChannelId) {
  auto& router = StorageRouter::Instance();
  auto info = MakeChannelInfo(42, "test", ChannelType::Spot, "OBTEST");
  router.RegisterChannel(42, info, true);

  DepthUpdateEvent event{};
  event.channel_id = 42;
  event.exchange_timestamp = 1000;
  event.bid_count = 2;
  event.ask_count = 2;
  event.bids[0] = {100.0, 1.0};
  event.bids[1] = {99.0, 2.0};
  event.asks[0] = {101.0, 1.0};
  event.asks[1] = {102.0, 2.0};

  router.RouteOrderbook(event, 500, info);
  router.FlushAndClose();
}

TEST_F(StorageRouterTest, RouteBookTickerUsesChannelId) {
  auto& router = StorageRouter::Instance();
  auto info = MakeChannelInfo(7, "test", ChannelType::Spot, "BTTEST");
  router.RegisterChannel(7, info, true);

  BookTickerEvent event{};
  event.channel_id = 7;
  event.exchange_timestamp = 2000;
  event.best_bid_price = 100.0;
  event.best_bid_qty = 1.0;
  event.best_ask_price = 101.0;
  event.best_ask_qty = 2.0;

  router.RouteBookTicker(event, info);
  router.FlushAndClose();
}

}  // namespace
}  // namespace sqc
