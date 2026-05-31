#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/binance/binance_common.h"
#include "src/exchange/binance/binance_spot.h"
#include "src/exchange/binance/binance_perpetual.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {
namespace {

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
  std::string json = R"({"e":"aggTrade","E":1716844800000000,"s":"BTCUSDT","a":123456789,"p":"50000.00","q":"1.50000000","f":123,"l":456,"T":1716844800001000,"m":true})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  TickData tick{};
  bool ok = binance::ParseTradeEvent(doc, tick, 1, "BTCUSDT");

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(tick.exchange_timestamp, 1716844800000000ULL);
    EXPECT_DOUBLE_EQ(tick.price, 50000.00);
    EXPECT_STREQ(tick.symbol, "BTCUSDT");
  }
}

TEST(BinanceParserTest, ParseHandlesBookTicker) {
  std::string json = R"({"e":"bookTicker","u":400900217,"s":"BNBUSDT","b":"25.19000000","B":"31.21000000","a":"25.20000000","A":"40.66000000","E":1716844800000000})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_spot::Parse(doc, 1, "BNBUSDT", EventType::BOOK_TICKER);

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

TEST(BinanceParserTest, ParseSpotBookTicker) {
  // Binance SPOT @bookTicker omits "e" and "E" fields.
  std::string json = R"({"u":400900217,"s":"BTCUSDT","b":"71621.60","B":"19.310","a":"71621.70","A":"2.554"})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_spot::Parse(doc, 1, "BTCUSDT", EventType::BOOK_TICKER);

  EXPECT_EQ(result.type, ParsedType::BOOK_TICKER);
  if (result.type == ParsedType::BOOK_TICKER) {
    EXPECT_DOUBLE_EQ(result.book_ticker.best_bid_price, 71621.60);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_bid_qty, 19.31);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_ask_price, 71621.70);
    EXPECT_DOUBLE_EQ(result.book_ticker.best_ask_qty, 2.554);
    EXPECT_EQ(result.book_ticker.channel_id, 1);
    EXPECT_STREQ(result.book_ticker.symbol, "BTCUSDT");
    EXPECT_GT(result.book_ticker.exchange_timestamp, 0ULL);
  }
}

TEST(BinanceParserTest, ParseHandlesTrade) {
  std::string json = R"({"e":"aggTrade","E":1716844800000000,"s":"BTCUSDT","a":123456789,"p":"50000.00","q":"1.50000000","m":true})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_spot::Parse(doc, 1, "BTCUSDT", EventType::TICK);

  EXPECT_EQ(result.type, ParsedType::TICK);
  if (result.type == ParsedType::TICK) {
    EXPECT_DOUBLE_EQ(result.tick.price, 50000.00);
    EXPECT_STREQ(result.tick.symbol, "BTCUSDT");
  }
}

TEST(BinanceParserTest, ParseSpotPartialDepth) {
  // Binance SPOT @depth<levels>@100ms — no "e" field, uses "bids"/"asks"/"lastUpdateId"
  std::string json = R"({"lastUpdateId":160,"bids":[["50000.00","1.50000000",[]],["49990.00","0.50000000",[]]],"asks":[["50010.00","2.00000000",[]],["50020.00","1.00000000",[]]]})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_spot::Parse(doc, 1, "BTCUSDT", EventType::DEPTH);

  EXPECT_EQ(result.type, ParsedType::DEPTH);
  if (result.type == ParsedType::DEPTH) {
    EXPECT_EQ(result.depth.last_update_id, 160ULL);
    EXPECT_STREQ(result.depth.symbol, "BTCUSDT");
    EXPECT_EQ(result.depth.bid_count, 2);
    EXPECT_EQ(result.depth.ask_count, 2);
    EXPECT_DOUBLE_EQ(result.depth.bids[0].price, 50000.00);
    EXPECT_DOUBLE_EQ(result.depth.bids[0].quantity, 1.5);
  }
}

TEST(BinanceParserTest, ParseFuturesDepthEvent) {
  // Perpetual timestamps are in milliseconds (unlike spot which uses microseconds via ?timeUnit=MICROSECOND)
  std::string json = R"({"e":"depthUpdate","E":1716844800000,"T":1716844800001,"s":"BTCUSDT","U":1001,"u":1005,"pu":999,"b":[["50000.00","1.50000000"]],"a":[["50010.00","2.00000000"]]})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  auto result = binance_perpetual::Parse(doc, 3, "BTCUSDT", EventType::DEPTH);

  EXPECT_EQ(result.type, ParsedType::DEPTH);
  if (result.type == ParsedType::DEPTH) {
    EXPECT_EQ(result.depth.last_update_id, 1005ULL);
    EXPECT_EQ(result.depth.bid_count, 1);
    EXPECT_EQ(result.depth.ask_count, 1);
    EXPECT_STREQ(result.depth.symbol, "BTCUSDT");
  }
}

}  // namespace
}  // namespace sqc
