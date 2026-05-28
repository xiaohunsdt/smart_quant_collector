#include <gtest/gtest.h>

#include <cstring>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/gateio/gateio_parser.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace {

// Real Gate.io futures.trades update message.
// Key points:
//  - price is a STRING
//  - size is a signed int: positive = buy, negative = sell
//  - trade object field order: id, create_time, create_time_ms, price, size, contract, is_internal
constexpr const char* kRealTradesJson = R"({
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

TEST(GateioParserTest, ParseRealTradeUpdate) {
  size_t size = std::strlen(kRealTradesJson);
  std::string padded(kRealTradesJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  TickData tick{};
  bool ok = gateio_parser::ParseTradeEvent(doc, tick, 2);

  EXPECT_TRUE(ok) << "ParseTradeEvent should succeed on real Gate.io trades data";
  EXPECT_EQ(tick.channel_id, 2u);
  EXPECT_STREQ(tick.symbol, "BTC_USDT");
  EXPECT_DOUBLE_EQ(tick.price, 96.4);
  EXPECT_DOUBLE_EQ(tick.quantity, 108.0);
  EXPECT_TRUE(tick.is_buyer_maker) << "negative size → sell → is_buyer_maker = true";
  EXPECT_EQ(tick.trade_id, 27753479u);
  EXPECT_EQ(tick.exchange_timestamp, 1545136464123000ULL);
}

// Real Gate.io futures.order_book "all" event (initial snapshot).
// Key points:
//  - event = "all" (not "update")
//  - contract is inside "result", not at top level
//  - bids/asks are objects: {"p": "string_price", "s": int_size}
constexpr const char* kRealOrderBookAllJson = R"({
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

TEST(GateioParserTest, ParseRealOrderBookAll) {
  size_t size = std::strlen(kRealOrderBookAllJson);
  std::string padded(kRealOrderBookAllJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  DepthUpdateEvent event{};
  bool ok = gateio_parser::ParseDepthEvent(doc, event, 7);

  EXPECT_TRUE(ok) << "ParseDepthEvent should accept 'all' events (initial snapshot)";
  EXPECT_EQ(event.channel_id, 7u);
  EXPECT_STREQ(event.symbol, "BTC_USDT");
  EXPECT_EQ(event.U, 113557080620u);
  EXPECT_EQ(event.u, 113557080620u);
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

}  // namespace
}  // namespace sqc
