#include <gtest/gtest.h>

#include <cstring>

#include "src/common/tick_data.h"
#include "src/storage/dolphindb_client.h"

namespace sqc {
namespace {

// --- Helpers ---

TickData MakeTick(uint64_t ts, const char* symbol, double price, double qty,
                  uint64_t trade_id = 1, uint32_t channel_id = 0,
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

// --- State machine tests (no server required) ---

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

TEST(DolphinDBClientTest, TableInsertWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  std::vector<TickData> batch = {MakeTick(1000, "BTCUSDT", 50000.0, 0.1)};
  EXPECT_FALSE(client.TableInsert("trades", batch));
}

TEST(DolphinDBClientTest, TableInsertEmptyBatchWhenDisconnectedReturnsFalse) {
  // Empty batch + not connected → returns false (connected_ check first).
  DolphinDBClient client;
  std::vector<TickData> empty;
  EXPECT_FALSE(client.TableInsert("trades", empty));
}

TEST(DolphinDBClientTest, UpsertWhenDisconnectedReturnsFalse) {
  DolphinDBClient client;
  std::vector<TickData> batch = {MakeTick(2000, "ETHUSDT", 3000.0, 1.5)};
  EXPECT_FALSE(client.Upsert("trades", batch));
}

TEST(DolphinDBClientTest, ReconnectWithoutServerFailsGracefully) {
  DolphinDBClient client;
  bool ok = client.Reconnect();
  EXPECT_FALSE(ok);
  EXPECT_FALSE(client.IsHealthy());
}

// --- SQL-building tests (always work, no server required) ---

TEST(DolphinDBClientTest, BuildInsertValuesSingleRow) {
  std::vector<TickData> batch = {MakeTick(1717000000000000ULL, "BTCUSDT",
                                          50000.5, 0.123)};
  auto sql = DolphinDBClient::BuildInsertValues("trades", batch);
  EXPECT_NE(sql.find("INSERT INTO trades"), std::string::npos);
  EXPECT_NE(sql.find("exchange_timestamp"), std::string::npos);
  EXPECT_NE(sql.find("local_diff"), std::string::npos);
  EXPECT_NE(sql.find("trade_id"), std::string::npos);
  EXPECT_NE(sql.find("price"), std::string::npos);
  EXPECT_NE(sql.find("quantity"), std::string::npos);
  EXPECT_NE(sql.find("channel_id"), std::string::npos);
  EXPECT_NE(sql.find("symbol"), std::string::npos);
  EXPECT_NE(sql.find("is_buyer_maker"), std::string::npos);
  EXPECT_NE(sql.find("VALUES"), std::string::npos);
  EXPECT_NE(sql.find("BTCUSDT"), std::string::npos);
  EXPECT_NE(sql.find("50000.5"), std::string::npos);
  EXPECT_NE(sql.find("0.123"), std::string::npos);
}

TEST(DolphinDBClientTest, BuildInsertValuesMultipleRows) {
  std::vector<TickData> batch = {
      MakeTick(1000, "BTCUSDT", 50000.0, 0.1, 1, 0, false),
      MakeTick(2000, "ETHUSDT", 3000.0, 1.5, 2, 1, true)};
  auto sql = DolphinDBClient::BuildInsertValues("trades", batch);
  EXPECT_NE(sql.find("(1000,"), std::string::npos);
  EXPECT_NE(sql.find("(2000,"), std::string::npos);
  EXPECT_NE(sql.find("true"), std::string::npos);
}

TEST(DolphinDBClientTest, BuildUpsertCallContainsUpsertKeyword) {
  std::vector<TickData> batch = {MakeTick(3000, "SOLUSDT", 150.0, 10.0)};
  auto sql = DolphinDBClient::BuildUpsertCall("trades", batch);
  EXPECT_NE(sql.find("upsert!("), std::string::npos);
  EXPECT_NE(sql.find("trades"), std::string::npos);
  EXPECT_NE(sql.find("`exchange_timestamp"), std::string::npos);
  EXPECT_NE(sql.find("`channel_id"), std::string::npos);
  EXPECT_NE(sql.find("[`channel_id,`exchange_timestamp,`trade_id]"),
            std::string::npos);
}

TEST(DolphinDBClientTest, BuildInsertValuesEscapesSymbol) {
  std::vector<TickData> batch = {MakeTick(1000, "BTCUSDT", 50000.0, 0.1)};
  auto sql = DolphinDBClient::BuildInsertValues("trades", batch);
  EXPECT_NE(sql.find("'BTCUSDT'"), std::string::npos);
}

// --- Integration tests (require a running DolphinDB server) ---
// To run: start DolphinDB on 127.0.0.1:8848, then uncomment:
//
// TEST(DolphinDBClientTest, Integration_ConnectAndInsert) {
//   DolphinDBClient client;
//   ASSERT_TRUE(client.Connect("127.0.0.1", 8848, "admin", "123"));
//   EXPECT_TRUE(client.IsHealthy());
//   std::vector<TickData> batch = {
//       MakeTick(1717000000000000ULL, "BTCUSDT", 50000.0, 0.1)};
//   EXPECT_TRUE(client.TableInsert("trades", batch));
//   client.Disconnect();
//   EXPECT_FALSE(client.IsHealthy());
// }

}  // namespace
}  // namespace sqc
