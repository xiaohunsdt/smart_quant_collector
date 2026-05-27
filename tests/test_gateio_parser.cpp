#include <gtest/gtest.h>

#include <cstring>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/gateio/gateio_parser.h"

namespace sqc {
namespace {

constexpr const char* kTickerJson = R"({
  "time": 1716844800.5,
  "contract": "BTC_USDT",
  "price": "50000.00",
  "size": "1500",
  "id": 123456789
})";

TEST(GateioParserTest, ParseTradeEvent) {
  std::string padded(kTickerJson);
  padded.resize(padded.size() + simdjson::SIMDJSON_PADDING, '\0');

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  auto err = parser.iterate(padded.data(), padded.size() - simdjson::SIMDJSON_PADDING,
                            padded.size()).get(doc);
  ASSERT_FALSE(err) << simdjson::error_message(err);

  TickData tick{};
  bool ok = gateio_parser::ParseTradeEvent(doc, tick, 2);

  EXPECT_TRUE(ok);
  EXPECT_EQ(tick.channel_id, 2);
  EXPECT_STREQ(tick.symbol, "BTC_USDT");
  EXPECT_EQ(tick.trade_id, 123456789);
}

}  // namespace
}  // namespace sqc
