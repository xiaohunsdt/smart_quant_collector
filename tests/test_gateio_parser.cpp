#include <gtest/gtest.h>

#include <cstring>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/gateio/gateio_parser.h"

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

}  // namespace
}  // namespace sqc
