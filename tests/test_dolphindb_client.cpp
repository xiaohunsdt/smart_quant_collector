#include <gtest/gtest.h>

#include <cstring>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/client/dolphindb_client.h"

namespace sqc {
namespace {

// --- Helpers ---

TickData MakeTick(uint64_t ts, const char* symbol, double price, double qty, uint64_t trade_id = 1, uint32_t channel_id = 0,
                  bool is_buyer_maker = false) {
  TickData t{};
  t.exchange_timestamp = ts;
  t.local_diff = 100;
  t.trade_id = trade_id;
  t.price = price;
  t.quantity = qty;
  t.channel_id = channel_id;
  std::strncpy(t.symbol, symbol, sizeof(t.symbol) - 1);
  t.is_buyer_maker = is_buyer_maker;
  return t;
}

DepthUpdateEvent MakeDepthEvent(uint32_t channel_id = 0) {
  DepthUpdateEvent ev{};
  ev.channel_id = channel_id;
  ev.exchange_timestamp = 1717000000000000ULL;
  ev.bid_count = 2;
  ev.ask_count = 2;
  ev.bids[0] = {50000.0, 1.0};
  ev.bids[1] = {49900.0, 2.0};
  ev.asks[0] = {50100.0, 1.5};
  ev.asks[1] = {50200.0, 3.0};
  return ev;
}

BookTickerEvent MakeBookTickerEvent(uint32_t channel_id = 0) {
  BookTickerEvent ev{};
  ev.channel_id = channel_id;
  ev.exchange_timestamp = 1717000000000000ULL;
  ev.local_diff = 100;
  ev.best_bid_price = 50000.0;
  ev.best_bid_qty = 1.0;
  ev.best_ask_price = 50100.0;
  ev.best_ask_qty = 1.5;
  return ev;
}

// ============================================================
// State machine tests (no server required)
// ============================================================

TEST(DolphinDBClientTest, DefaultStateIsNotConnected) {
  DolphinDBClient client;
  EXPECT_FALSE(client.IsHealthy());
}

TEST(DolphinDBClientTest, ConnectWithoutServerFailsGracefully) {
  DolphinDBClient client;
  bool ok = client.Connect("127.0.0.1", 19999, "admin", "123");
  EXPECT_FALSE(ok);
  EXPECT_FALSE(client.IsHealthy());
}

TEST(DolphinDBClientTest, TableInsertTradesWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  std::vector<TickData> batch = {MakeTick(1000, "BTCUSDT", 50000.0, 0.1)};
  EXPECT_FALSE(client.TableInsertTrades(batch));
}

TEST(DolphinDBClientTest, TableInsertTradesEmptyBatch) {
  DolphinDBClient client;
  std::vector<TickData> empty;
  EXPECT_FALSE(client.TableInsertTrades(empty));
}

TEST(DolphinDBClientTest, TableInsertOrderbookWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  DepthUpdateEvent ev = MakeDepthEvent();
  EXPECT_FALSE(client.TableInsertOrderbook(ev, 100));
}

TEST(DolphinDBClientTest, TableInsertBookTickerWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  BookTickerEvent ev = MakeBookTickerEvent();
  EXPECT_FALSE(client.TableInsertBookTicker(ev));
}

TEST(DolphinDBClientTest, ReconnectWithoutServerFailsGracefully) {
  DolphinDBClient client;
  bool ok = client.Reconnect();
  EXPECT_FALSE(ok);
  EXPECT_FALSE(client.IsHealthy());
}

// ============================================================
// Unit tests (pure logic, no server required)
// ============================================================
// Note: timestamp validation (UsecToDateInt / IsValidTradeDate /
// IsValidExchangeTimestamp) is tested in test_timestamp_util.cpp — the DolphinDB
// client no longer carries those wrappers (they were trivial delegations to
// timestamp_util). Tests below cover genuine DolphinDBClient behavior.

TEST(DolphinDBClientTest, RegisterChannelResolvesMeta) {
  DolphinDBClient client;
  client.RegisterChannel(0, "binance", "perpetual", "BTCUSDT", 10);
  client.RegisterChannel(1, "gateio", "spot", "ETH_USDT", 5);
  SUCCEED();
}

TEST(DolphinDBClientTest, RegisterChannelDefaultDepthLevel) {
  DolphinDBClient client;
  // depth_level defaults to 0 when not provided
  client.RegisterChannel(0, "binance", "spot", "BTCUSDT");
  SUCCEED();
}

TEST(DolphinDBClientTest, ValidateSchemaWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  EXPECT_FALSE(client.ValidateSchema());
}

TEST(DolphinDBClientTest, DestroyWritersNoCrashOnEmpty) {
  DolphinDBClient client;
  // DestroyWriters is called internally by Disconnect; should not crash.
  client.Disconnect();
  SUCCEED();
}

// ============================================================
// Integration tests (require a running DolphinDB server)
// ============================================================
// To run: start DolphinDB on 127.0.0.1:8848, ensure config has
// auto_init_schema=true, then uncomment the tests below.
//
// TEST(DolphinDBClientTest, Integration_ConnectAndInsertTrades) {
//   DolphinDBClient client;
//   ASSERT_TRUE(client.Connect("127.0.0.1", 8848, "admin", "123456"));
//   EXPECT_TRUE(client.IsHealthy());
//   EXPECT_TRUE(client.ValidateSchema());
//   client.RegisterChannel(0, "binance", "perpetual", "BTCUSDT", 10);
//
//   std::vector<TickData> batch = {
//       MakeTick(1717000000000000ULL, "BTCUSDT", 50000.0, 0.1)};
//   EXPECT_TRUE(client.TableInsertTrades(batch));
//
//   client.Disconnect();
//   EXPECT_FALSE(client.IsHealthy());
// }

}  // namespace
}  // namespace sqc
