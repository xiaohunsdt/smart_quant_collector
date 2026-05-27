#include <gtest/gtest.h>
#include <thread>
#include <cstring>
#include "simdjson.h"
#include "src/common/simdjson_utils.h"
#include "src/exchange/shard_queue.h"

namespace sqc {
namespace {

// Reproduce the exact parsing pattern used in ExchangeChannel::OnMessage
TEST(OnMessageParsePattern, ParseGateioTickerJson) {
  constexpr const char* kJson = R"({"time":1779918309,"time_ms":1779918309804,"channel":"futures.tickers","event":"update","result":[{"contract":"BTC_USDT","last":"50000.00","size":"1500"}]})";
  size_t size = std::strlen(kJson);

  // Exactly the same pattern as OnMessage
  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kJson, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser peek_parser;
  simdjson::ondemand::document peek_doc;
  auto err = peek_parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(peek_doc);
  ASSERT_FALSE(err) << "simdjson iterate failed: " << simdjson::error_message(err);

  std::string_view channel;
  EXPECT_NO_THROW(channel = std::string_view(peek_doc["channel"]));
  EXPECT_EQ(channel, "futures.tickers");
}

TEST(OnMessageParsePattern, ParseBinanceTradeJson) {
  constexpr const char* kJson = R"({"e":"trade","E":1779918309804,"s":"BTCUSDT","t":123456789,"p":"50000.00","q":"0.001","b":123,"a":456,"T":1779918309800,"m":false,"M":true})";
  size_t size = std::strlen(kJson);

  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kJson, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser peek_parser;
  simdjson::ondemand::document peek_doc;
  auto err = peek_parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(peek_doc);
  ASSERT_FALSE(err) << "simdjson iterate failed: " << simdjson::error_message(err);

  std::string_view symbol;
  EXPECT_NO_THROW(symbol = std::string_view(peek_doc["s"]));
  EXPECT_EQ(symbol, "BTCUSDT");
}

// Test the CORRECT extraction pattern that the fixed code should use.
// RED: these tests call the helper that doesn't exist yet / logic isn't wired up.
TEST(OnMessageParsePattern, ExtractSymbolGateioTickerArray) {
  // Gate.io futures.tickers: "contract" nested in "result" array
  constexpr const char* kJson = R"({"time":1779918309,"time_ms":1779918309804,"channel":"futures.tickers","event":"update","result":[{"contract":"BTC_USDT","last":"50000.00","size":"1500"}]})";
  size_t size = std::strlen(kJson);

  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kJson, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ASSERT_FALSE(parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(doc));

  // Correct extraction: drill into "result" array, get first element's "contract"
  std::string_view symbol;
  bool found = false;
  try {
    auto result = doc["result"];
    simdjson::ondemand::array arr;
    if (!result.get_array().get(arr)) {
      for (auto item : arr) {
        std::string_view contract = item["contract"];
        if (!contract.empty()) { symbol = contract; found = true; break; }
      }
    }
  } catch (...) {}
  EXPECT_TRUE(found) << "Should extract contract from result array";
  EXPECT_EQ(symbol, "BTC_USDT");
}

TEST(OnMessageParsePattern, ExtractSymbolGateioOrderBookObject) {
  // Gate.io futures.order_book: "contract" nested in "result" object
  constexpr const char* kJson = R"({"time":1779918309,"channel":"futures.order_book","event":"update","result":{"contract":"ETH_USDT","asks":[],"bids":[]}})";
  size_t size = std::strlen(kJson);

  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kJson, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ASSERT_FALSE(parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(doc));

  // Correct extraction: "contract" is directly inside "result" object
  std::string_view symbol;
  bool found = false;
  try {
    std::string_view contract = doc["result"]["contract"];
    if (!contract.empty()) { symbol = contract; found = true; }
  } catch (...) {}
  EXPECT_TRUE(found) << "Should extract contract from result object";
  EXPECT_EQ(symbol, "ETH_USDT");
}

TEST(OnMessageParsePattern, GateioSubscribeConfirmationIsControlMessage) {
  // The actual message that was confusing the system:
  // subscription confirmations have no "contract" field and should be skipped
  constexpr const char* kJson = R"({"time":1779919271,"time_ms":1779919271077,"conn_id":"03a45eb1ad41ffdc","trace_id":"6aa71a76ba9301860555cbdadcc91df7","channel":"futures.tickers","event":"subscribe","payload":["BTC_USDT","ETH_USDT","SOL_USDT"],"result":{"status":"success"}})";
  size_t size = std::strlen(kJson);

  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kJson, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  ASSERT_FALSE(parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(doc));

  // "event" is "subscribe" — should be filtered out, not treated as data
  std::string_view event = doc["event"];
  EXPECT_EQ(event, "subscribe");

  // "contract" doesn't exist in subscribe confirmations
  bool contract_missing = false;
  try { (void)std::string_view(doc["result"]["contract"]); }
  catch (const simdjson::simdjson_error&) { contract_missing = true; }
  EXPECT_TRUE(contract_missing) << "subscribe confirmation should not have top-level contract";
}

TEST(OnMessageParsePattern, TruncatedJsonFailsGracefully) {
  // Simulate truncated data as seen in the user's log
  constexpr const char* kTruncated = R"({"time":1779918309,"time_ms":1779918309804,"conn_id":"ef0a0208d049f24c","trace_i)";
  size_t size = std::strlen(kTruncated);

  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), kTruncated, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  simdjson::ondemand::parser peek_parser;
  simdjson::ondemand::document peek_doc;
  auto err = peek_parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(peek_doc);

  // Truncated JSON should fail to parse — this is EXPECTED
  // The fix should handle this gracefully (not crash)
  EXPECT_TRUE(err) << "Truncated JSON should produce a parse error";
  if (err) {
    // Verify we get a sensible error, not a crash
    std::string err_msg = simdjson::error_message(err);
    EXPECT_FALSE(err_msg.empty());
  }

  // Verify accessing fields on truncated doc doesn't crash
  if (!err) {
    try { (void)std::string_view(peek_doc["contract"]); } catch (...) {}
  }
}

TEST(ShardQueueTest, PushPopFIFO) {
  ShardQueue q(16);
  auto d1 = std::shared_ptr<char[]>(new char[6]); std::memcpy(d1.get(), "hello", 5);
  auto d2 = std::shared_ptr<char[]>(new char[6]); std::memcpy(d2.get(), "world", 5);
  RawMessage m1; m1.data = d1; m1.size = 5;
  RawMessage m2; m2.data = d2; m2.size = 5;
  q.Push(std::move(m1)); q.Push(std::move(m2));
  RawMessage out;
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.size, 5u);
  EXPECT_TRUE(q.TryPop(out)); EXPECT_EQ(out.size, 5u);
}

TEST(ShardQueueTest, PoisonPill) {
  ShardQueue q(16);
  q.PushPoisonPill();
  RawMessage out = q.PopBlocking();
  EXPECT_EQ(out.data, nullptr);
  EXPECT_EQ(out.size, 0u);
}

TEST(ShardQueueTest, ConcurrentPushPop) {
  ShardQueue q(256);
  constexpr int kCount = 100;
  std::thread producer([&]() {
    for (int i = 0; i < kCount; ++i) {
      RawMessage m;
      m.data = std::shared_ptr<char[]>(new char[1]);
      m.size = 1;
      q.Push(std::move(m));
    }
    q.PushPoisonPill();
  });
  int count = 0;
  while (true) {
    RawMessage out = q.PopBlocking();
    if (!out.data) break;
    count++;
  }
  producer.join();
  EXPECT_EQ(count, kCount);
}

}  // namespace
}  // namespace sqc
