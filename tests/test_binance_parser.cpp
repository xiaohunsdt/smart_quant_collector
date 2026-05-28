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
  std::string json = R"({"e":"aggTrade","E":1716844800000,"s":"BTCUSDT","a":123456789,"p":"50000.00","q":"1.50000000","f":123,"l":456,"T":1716844800001,"m":true})";

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

TEST(BinanceParserTest, ParseDepthEventSpotNoPuField) {
  // Binance SPOT depthUpdate — no "pu" field. Computed from first_update_id - 1.
  std::string json = R"({"e":"depthUpdate","E":1716844800000,"s":"BTCUSDT","U":1001,"u":1005,"b":[["50000.00","1.50000000"],["49990.00","0.50000000"]],"a":[["50010.00","2.00000000"],["50020.00","1.00000000"]]})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  DepthUpdateEvent depth{};
  bool ok = binance_parser::ParseDepthEvent(doc, depth, 1);

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(depth.exchange_timestamp, 1716844800000000ULL);
    EXPECT_STREQ(depth.symbol, "BTCUSDT");
    EXPECT_EQ(depth.first_update_id, 1001ULL);
    EXPECT_EQ(depth.last_update_id, 1005ULL);
    EXPECT_EQ(depth.prev_last_update_id, 1000ULL);  // first_update_id - 1
    EXPECT_EQ(depth.channel_id, 1);
    EXPECT_EQ(depth.bid_count, 2);
    if (depth.bid_count >= 2) {
      EXPECT_DOUBLE_EQ(depth.bids[0].price, 50000.00);
      EXPECT_DOUBLE_EQ(depth.bids[0].quantity, 1.5);
      EXPECT_DOUBLE_EQ(depth.bids[1].price, 49990.00);
      EXPECT_DOUBLE_EQ(depth.bids[1].quantity, 0.5);
    }
    EXPECT_EQ(depth.ask_count, 2);
    if (depth.ask_count >= 2) {
      EXPECT_DOUBLE_EQ(depth.asks[0].price, 50010.00);
      EXPECT_DOUBLE_EQ(depth.asks[0].quantity, 2.0);
      EXPECT_DOUBLE_EQ(depth.asks[1].price, 50020.00);
      EXPECT_DOUBLE_EQ(depth.asks[1].quantity, 1.0);
    }
  }
}

TEST(BinanceParserTest, ParseDepthEventFuturesHasPuField) {
  // Binance USDⓈ-M Futures depthUpdate — HAS "pu" field.
  std::string json = R"({"e":"depthUpdate","E":1716844800000,"T":1716844800001,"s":"BTCUSDT","U":1001,"u":1005,"pu":999,"b":[["50000.00","1.50000000"]],"a":[["50010.00","2.00000000"]]})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  DepthUpdateEvent depth{};
  bool ok = binance_parser::ParseDepthEvent(doc, depth, 3);

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(depth.first_update_id, 1001ULL);
    EXPECT_EQ(depth.last_update_id, 1005ULL);
    EXPECT_EQ(depth.prev_last_update_id, 999ULL);  // from "pu" field
    EXPECT_EQ(depth.channel_id, 3);
    EXPECT_EQ(depth.bid_count, 1);
    EXPECT_EQ(depth.ask_count, 1);
  }
}

TEST(BinanceParserTest, ParseDepthEventFirstUpdateIdZero) {
  // first_update_id == 0, no "pu" → prev_last_update_id should stay 0.
  std::string json = R"({"e":"depthUpdate","E":1716844800000,"s":"BTCUSDT","U":0,"u":0,"b":[],"a":[]})";

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ParseDoc(parser, json, doc);

  DepthUpdateEvent depth{};
  bool ok = binance_parser::ParseDepthEvent(doc, depth, 2);

  EXPECT_TRUE(ok);
  if (ok) {
    EXPECT_EQ(depth.prev_last_update_id, 0ULL);
  }
}

TEST(BinanceParserTest, ParseMessageHandlesTradeEvent) {
  // aggTrade event parsed end-to-end via ParseMessage.
  std::string json = R"({"e":"aggTrade","E":1716844800000,"s":"BTCUSDT","a":123456789,"p":"50000.00","q":"1.50000000","m":true})";

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
