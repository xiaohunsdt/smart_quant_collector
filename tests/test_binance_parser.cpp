#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <cstdio>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/binance/binance_parser.h"

namespace sqc {
namespace {

TEST(BinanceParserTest, ParseTradeEvent) {
  fprintf(stderr, "=== test start ===\n");
  std::string json = R"({"e":"trade","E":1716844800000,"s":"BTCUSDT","t":123456789,"p":"50000.00","q":"1.50000000","b":123,"a":456,"T":1716844800001,"m":true,"M":true})";
  fprintf(stderr, "json built, size=%zu\n", json.size());
  json.resize(json.size() + simdjson::SIMDJSON_PADDING, '\0');
  fprintf(stderr, "resized to %zu\n", json.size());

  simdjson::ondemand::parser parser;
  fprintf(stderr, "parser created\n");
  simdjson::ondemand::document doc;
  fprintf(stderr, "doc declared, iterating...\n");
  auto error = parser.iterate(json.data(), json.size() - simdjson::SIMDJSON_PADDING,
                              json.size()).get(doc);
  fprintf(stderr, "iterate done, error=%d\n", (int)error);
  if (error) {
    fprintf(stderr, "simdjson error: %s\n", simdjson::error_message(error));
    FAIL() << "simdjson iterate error: " << simdjson::error_message(error);
  }
  fprintf(stderr, "doc ready\n");

  TickData tick{};
  fprintf(stderr, "calling ParseTradeEvent...\n");
  bool ok = binance_parser::ParseTradeEvent(doc, tick, 1);
  fprintf(stderr, "ParseTradeEvent returned %d\n", ok);

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(tick.exchange_timestamp, 1716844800000000ULL);
    EXPECT_DOUBLE_EQ(tick.price, 50000.00);
    fprintf(stderr, "=== test passed ===\n");
  }
}

}  // namespace
}  // namespace sqc
