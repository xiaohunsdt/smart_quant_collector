#include <gtest/gtest.h>

#include <cstring>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/gateio/gateio_parser.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace {

// Real Gate.io futures.tickers update message (all numeric fields are strings)
constexpr const char* kRealTickerJson = R"({
  "time": 1779916108,
  "time_ms": 1779916108315,
  "channel": "futures.tickers",
  "event": "update",
  "result": [
    {
      "contract": "BTC_USDT",
      "last": "75180.8",
      "change_percentage": "-1.0246",
      "total_size": "616999728",
      "volume_24h": "562890187",
      "volume_24h_base": "56289.0187",
      "volume_24h_quote": "4231853464",
      "volume_24h_settle": "4231853464",
      "mark_price": "75180.8",
      "funding_rate": "0.000094",
      "funding_rate_indicative": "0.000094",
      "index_price": "75210.0",
      "quanto_base_rate": "",
      "low_24h": "74635.0",
      "high_24h": "76143.0",
      "price_type": "last",
      "change_from": "24h",
      "change_price": "-778.3",
      "t": 1779916107555
    }
  ]
})";

TEST(GateioParserTest, ParseRealTickerUpdate) {
  size_t size = std::strlen(kRealTickerJson);
  std::string padded(kRealTickerJson);
  padded.resize(size + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), size, size + simdjson::SIMDJSON_PADDING).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  TickData tick{};
  bool ok = gateio_parser::ParseTradeEvent(doc, tick, 2);

  EXPECT_TRUE(ok) << "ParseTradeEvent should succeed on real Gate.io ticker data";
  EXPECT_EQ(tick.channel_id, 2u);
  EXPECT_STREQ(tick.symbol, "BTC_USDT");
  EXPECT_DOUBLE_EQ(tick.price, 75180.8);
  EXPECT_GT(tick.quantity, 0.0);
  EXPECT_GT(tick.exchange_timestamp, 0u);
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
