#include <gtest/gtest.h>

#include <cstring>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/gateio/gateio_perpetual.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace {

// ---- Futures test data ----

// Real futures.trades
constexpr const char* kFuturesTradesJson = R"({
  "time": 1779916108,
  "time_ms": 1779916108315,
  "channel": "futures.trades",
  "event": "update",
  "result": [
    {
      "id": 27753479,
      "create_time": 1545136464,
      "create_time_ms": 1545136464123,
      "price": "96.4",
      "size": -108,
      "contract": "BTC_USDT",
      "is_internal": false
    }
  ]
})";

TEST(GateioParserTest, ParseFuturesTrade) {
  size_t size = std::strlen(kFuturesTradesJson);
  std::string padded(kFuturesTradesJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  TickData tick{};
  bool ok = gateio_perpetual::ParseTradeEvent(doc, tick, 2);

  EXPECT_TRUE(ok);
  EXPECT_EQ(tick.channel_id, 2u);
  EXPECT_STREQ(tick.symbol, "BTC_USDT");
  EXPECT_DOUBLE_EQ(tick.price, 96.4);
  EXPECT_DOUBLE_EQ(tick.quantity, 108.0);
  EXPECT_TRUE(tick.is_buyer_maker);
  EXPECT_EQ(tick.trade_id, 27753479u);
  EXPECT_EQ(tick.exchange_timestamp, 1545136464123000ULL);
}

// Real futures.order_book (all event, snapshot)
constexpr const char* kFuturesOrderBookAllJson = R"({
  "time": 1779923753,
  "time_ms": 1779923753576,
  "channel": "futures.order_book",
  "event": "all",
  "result": {
    "t": 1779923753557,
    "id": 113557080620,
    "contract": "BTC_USDT",
    "asks": [
      {"p": "74486.1", "s": 138237},
      {"p": "74486.2", "s": 130926},
      {"p": "74486.6", "s": 751}
    ],
    "bids": [
      {"p": "74486", "s": 29059},
      {"p": "74485.4", "s": 147},
      {"p": "74485.3", "s": 1491}
    ],
    "l": "10"
  }
})";

TEST(GateioParserTest, ParseFuturesOrderBookAll) {
  size_t size = std::strlen(kFuturesOrderBookAllJson);
  std::string padded(kFuturesOrderBookAllJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  DepthUpdateEvent event{};
  bool ok = gateio_perpetual::ParseDepthEvent(doc, event, 7);

  EXPECT_TRUE(ok);
  EXPECT_EQ(event.channel_id, 7u);
  EXPECT_STREQ(event.symbol, "BTC_USDT");
  EXPECT_EQ(event.first_update_id, 113557080620u);
  EXPECT_EQ(event.last_update_id, 113557080620u);
  EXPECT_EQ(event.exchange_timestamp, 1779923753557000ULL);

  ASSERT_EQ(event.bid_count, 3u);
  EXPECT_DOUBLE_EQ(event.bids[0].price, 74486.0);
  EXPECT_DOUBLE_EQ(event.bids[0].quantity, 29059.0);
  EXPECT_DOUBLE_EQ(event.bids[1].price, 74485.4);
  EXPECT_DOUBLE_EQ(event.bids[1].quantity, 147.0);
  EXPECT_DOUBLE_EQ(event.bids[2].price, 74485.3);
  EXPECT_DOUBLE_EQ(event.bids[2].quantity, 1491.0);

  ASSERT_EQ(event.ask_count, 3u);
  EXPECT_DOUBLE_EQ(event.asks[0].price, 74486.1);
  EXPECT_DOUBLE_EQ(event.asks[0].quantity, 138237.0);
  EXPECT_DOUBLE_EQ(event.asks[1].price, 74486.2);
  EXPECT_DOUBLE_EQ(event.asks[1].quantity, 130926.0);
  EXPECT_DOUBLE_EQ(event.asks[2].price, 74486.6);
  EXPECT_DOUBLE_EQ(event.asks[2].quantity, 751.0);
}

// Real futures.order_book_update (incremental)
// v4 format: U/u for update IDs, s for symbol, b/a with object entries {p, s}.
constexpr const char* kFuturesOrderBookUpdateJson = R"({
  "time": 1779923753,
  "time_ms": 1779923753576,
  "channel": "futures.order_book_update",
  "event": "update",
  "result": {
    "t": 1779923753557,
    "s": "BTC_USDT",
    "U": 2517661101,
    "u": 2517661113,
    "b": [
      {"p": "54672.1", "s": 0},
      {"p": "54664.5", "s": 58794}
    ],
    "a": [
      {"p": "54743.6", "s": 0},
      {"p": "54742", "s": 95}
    ]
  }
})";

TEST(GateioParserTest, ParseFuturesOrderBookUpdate) {
  size_t size = std::strlen(kFuturesOrderBookUpdateJson);
  std::string padded(kFuturesOrderBookUpdateJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  DepthUpdateEvent event{};
  bool ok = gateio_perpetual::ParseDepthUpdateEvent(doc, event, 7);

  EXPECT_TRUE(ok);
  EXPECT_EQ(event.channel_id, 7u);
  EXPECT_STREQ(event.symbol, "BTC_USDT");
  EXPECT_EQ(event.first_update_id, 2517661101u);
  EXPECT_EQ(event.last_update_id, 2517661113u);
  EXPECT_EQ(event.prev_last_update_id, 2517661100u);
  EXPECT_EQ(event.exchange_timestamp, 1779923753557000ULL);

  ASSERT_EQ(event.bid_count, 2u);
  EXPECT_DOUBLE_EQ(event.bids[0].price, 54672.1);
  EXPECT_DOUBLE_EQ(event.bids[0].quantity, 0.0);
  EXPECT_DOUBLE_EQ(event.bids[1].price, 54664.5);
  EXPECT_DOUBLE_EQ(event.bids[1].quantity, 58794.0);

  ASSERT_EQ(event.ask_count, 2u);
  EXPECT_DOUBLE_EQ(event.asks[0].price, 54743.6);
  EXPECT_DOUBLE_EQ(event.asks[0].quantity, 0.0);
  EXPECT_DOUBLE_EQ(event.asks[1].price, 54742.0);
  EXPECT_DOUBLE_EQ(event.asks[1].quantity, 95.0);
}

// Real futures.book_ticker
// v4 format: top-level b/B/a/A (no nested "book" object).
constexpr const char* kFuturesBookTickerJson = R"({
  "time": 1615366379,
  "time_ms": 1615366379123,
  "channel": "futures.book_ticker",
  "event": "update",
  "result": {
    "t": 1615366379123,
    "u": 2517661076,
    "s": "BTC_USDT",
    "b": "54696.6",
    "B": 37000,
    "a": "54696.7",
    "A": 47061
  }
})";

TEST(GateioParserTest, ParseFuturesBookTicker) {
  size_t size = std::strlen(kFuturesBookTickerJson);
  std::string padded(kFuturesBookTickerJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  BookTickerEvent event{};
  bool ok = gateio_perpetual::ParseBookTickerEvent(doc, event, 7);

  EXPECT_TRUE(ok);
  EXPECT_EQ(event.channel_id, 7u);
  EXPECT_STREQ(event.symbol, "BTC_USDT");
  EXPECT_DOUBLE_EQ(event.best_bid_price, 54696.6);
  EXPECT_DOUBLE_EQ(event.best_bid_qty, 37000.0);
  EXPECT_DOUBLE_EQ(event.best_ask_price, 54696.7);
  EXPECT_DOUBLE_EQ(event.best_ask_qty, 47061.0);
  EXPECT_EQ(event.exchange_timestamp, 1615366379123000ULL);
}

// ---- ParseMessage dispatch tests ----

TEST(GateioParserTest, ParseMessageDispatchesToDepthUpdate) {
  size_t size = std::strlen(kFuturesOrderBookUpdateJson);
  std::string padded(kFuturesOrderBookUpdateJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  auto result = gateio_perpetual::ParseMessage(doc, 7);
  EXPECT_EQ(result.type, ParsedType::DEPTH);
  EXPECT_EQ(result.depth.first_update_id, 2517661101u);
}

TEST(GateioParserTest, ParseMessageDispatchesToBookTicker) {
  size_t size = std::strlen(kFuturesBookTickerJson);
  std::string padded(kFuturesBookTickerJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  auto result = gateio_perpetual::ParseMessage(doc, 7);
  EXPECT_EQ(result.type, ParsedType::BOOK_TICKER);
  EXPECT_DOUBLE_EQ(result.book_ticker.best_bid_price, 54696.6);
}

// order_book_update skips "subscribe" event
TEST(GateioParserTest, ParseDepthUpdateSkipsSubscribeEvent) {
  constexpr const char* kSubscribeJson = R"({
    "time": 1779923753,
    "time_ms": 1779923753576,
    "channel": "futures.order_book_update",
    "event": "subscribe",
    "result": null
  })";

  size_t size = std::strlen(kSubscribeJson);
  std::string padded(kSubscribeJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  DepthUpdateEvent event{};
  bool ok = gateio_perpetual::ParseDepthUpdateEvent(doc, event, 7);
  EXPECT_FALSE(ok) << "subscribe event should be skipped";
}

}  // namespace
}  // namespace sqc
