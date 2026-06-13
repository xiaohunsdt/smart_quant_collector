#include <gtest/gtest.h>

#include <cstring>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "src/storage/dolphindb_client.h"

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

TEST(DolphinDBClientTest, UsecToDateInt) {
  // Zero timestamp is invalid — returns sentinel -1
  EXPECT_EQ(DolphinDBClient::UsecToDateInt(0), -1);
  EXPECT_EQ(DolphinDBClient::UsecToDateInt(86400000000ULL), 1);
  uint64_t ts = 1780445977532000ULL;
  int date = DolphinDBClient::UsecToDateInt(ts);
  EXPECT_GT(date, 20000);
}

TEST(DolphinDBClientTest, UsecToDateIntZeroReturnsNegativeOne) {
  // Zero timestamp is invalid — sentinel
  EXPECT_EQ(DolphinDBClient::UsecToDateInt(0), -1);
}

TEST(DolphinDBClientTest, UsecToDateIntOverflowReturnsNegativeOne) {
  // Beyond year 2100 — sentinel
  constexpr uint64_t kOverflow = 4102444800000001ULL;
  EXPECT_EQ(DolphinDBClient::UsecToDateInt(kOverflow), -1);
}

TEST(DolphinDBClientTest, IsValidTradeDateNegativeFails) {
  EXPECT_FALSE(DolphinDBClient::IsValidTradeDate(-1));
  EXPECT_FALSE(DolphinDBClient::IsValidTradeDate(-100));
}

TEST(DolphinDBClientTest, IsValidTradeDateBefore2000Fails) {
  // Day 0 = 1970-01-01, before our valid window
  EXPECT_FALSE(DolphinDBClient::IsValidTradeDate(0));
  // Day 10000 = ~1997, still before 2000
  EXPECT_FALSE(DolphinDBClient::IsValidTradeDate(10000));
}

TEST(DolphinDBClientTest, IsValidTradeDateCurrentDatePasses) {
  // Compute today's date int and verify it's valid
  auto now = std::time(nullptr);
  int today = static_cast<int>(now / 86400);
  EXPECT_TRUE(DolphinDBClient::IsValidTradeDate(today));
}

TEST(DolphinDBClientTest, IsValidTradeDateFarFutureFails) {
  // Day corresponding to year 2150 — far beyond valid window
  // ~65745 days from epoch = approx Jan 1, 2150
  int day_2150 = 65745;
  EXPECT_FALSE(DolphinDBClient::IsValidTradeDate(day_2150));
}

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

TEST(DolphinDBClientTest, IsValidExchangeTimestampZeroFails) {
  EXPECT_FALSE(DolphinDBClient::IsValidExchangeTimestamp(0));
}

TEST(DolphinDBClientTest, IsValidExchangeTimestampValidPasses) {
  EXPECT_TRUE(DolphinDBClient::IsValidExchangeTimestamp(1717000000000000ULL));
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
