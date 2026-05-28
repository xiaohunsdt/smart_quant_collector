#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/binance/binance_parser.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {
namespace {

// Helper: pads the string and iterates it into a document.
// The caller MUST keep the string (json) alive for the lifetime of doc
// because simdjson ondemand borrows the input buffer.
void ParseDoc(simdjson::ondemand::parser& parser, std::string& json,
              simdjson::ondemand::document& doc) {
  json.resize(json.size() + simdjson::SIMDJSON_PADDING, '\0');
  auto error = parser.iterate(json.data(), json.size() - simdjson::SIMDJSON_PADDING,
                              json.size()).get(doc);
  if (error) {
    FAIL() << "simdjson iterate error: " << simdjson::error_message(error);
  }
}

TEST(BinanceParserTest, ParseTradeEvent) {
  std::string json = R"({"e":"trade","E":1716844800000,"s":"BTCUSDT","t":123456789,"p":"50000.00","q":"1.50000000","b":123,"a":456,"T":1716844800001,"m":true,"M":true})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  TickData tick{};
  bool ok = binance_parser::ParseTradeEvent(doc, tick, 1);

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(tick.exchange_timestamp, 1716844800000000ULL);
    EXPECT_DOUBLE_EQ(tick.price, 50000.00);
  }
}

TEST(BinanceParserTest, ParseMessageSkipsMessageWithoutEventField) {
  // Binance subscription confirmation: {"result":null,"id":1}
  // These messages lack the "e" field and should be silently skipped.
  std::string json = R"({"result":null,"id":1})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_parser::ParseMessage(doc, 1);

  EXPECT_EQ(result.type, ParsedType::NONE);
}

TEST(BinanceParserTest, ParseMessageHandlesBookTickerEvent) {
  std::string json = R"({"e":"bookTicker","u":400900217,"s":"BNBUSDT","b":"25.19000000","B":"31.21000000","a":"25.20000000","A":"40.66000000","E":1716844800000})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_parser::ParseMessage(doc, 1);

  EXPECT_EQ(result.type, ParsedType::BOOK_TICKER);
  if (result.type == ParsedType::BOOK_TICKER) {
    EXPECT_DOUBLE_EQ(result.book_ticker.best_bid_price, 25.19);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_bid_qty, 31.21);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_ask_price, 25.20);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_ask_qty, 40.66);
    EXPECT_EQ(result.book_ticker.channel_id, 1);
    EXPECT_STREQ(result.book_ticker.symbol, "BNBUSDT");
  }
}

TEST(BinanceParserTest, ParseMessageSkipsMessageWithUnknownEventType) {
  // A message with an "e" field but an event type we don't handle.
  std::string json = R"({"e":"unknownEvent","s":"BNBUSDT","x":1})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_parser::ParseMessage(doc, 1);

  EXPECT_EQ(result.type, ParsedType::NONE);
}

TEST(BinanceParserTest, ParseMessageHandlesTradeEvent) {
  // Trade event parsed end-to-end via ParseMessage.
  std::string json = R"({"e":"trade","E":1716844800000,"s":"BTCUSDT","t":123456789,"p":"50000.00","q":"1.50000000","m":true})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_parser::ParseMessage(doc, 1);

  EXPECT_EQ(result.type, ParsedType::TICK);
  if (result.type == ParsedType::TICK) {
    EXPECT_DOUBLE_EQ(result.tick.price, 50000.00);
  }
}

}  // namespace
}  // namespace sqc
