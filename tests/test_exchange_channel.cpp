#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "src/exchange/channel_spec.h"
#include "src/exchange/exchange_channel.h"

namespace sqc {
namespace {

TEST(ExchangeChannelTest, OrderBookSubscribePayloadSingleSymbol) {
  std::vector<SymbolSpec> symbols = {
      {"ETH_USDT", true, 20, true},
  };

  std::string payload = BuildGateioOrderBookSubscribePayload(symbols, 1779928915);

  EXPECT_NE(payload.find("\"ETH_USDT\""), std::string::npos);
  EXPECT_NE(payload.find("futures.order_book"), std::string::npos);
  EXPECT_NE(payload.find("\"subscribe\""), std::string::npos);
  // Must NOT contain depth_level or interval as separate payload elements
  EXPECT_EQ(payload.find("\"20\""), std::string::npos);
  EXPECT_EQ(payload.find("\"0\""), std::string::npos);
}

TEST(ExchangeChannelTest, OrderBookSubscribePayloadMultipleSymbols) {
  std::vector<SymbolSpec> symbols = {
      {"ETH_USDT", true, 20, true},
      {"SOL_USDT", true, 20, true},
  };

  std::string payload = BuildGateioOrderBookSubscribePayload(symbols, 1779928915);

  // Both contract names must appear
  EXPECT_NE(payload.find("\"ETH_USDT\""), std::string::npos);
  EXPECT_NE(payload.find("\"SOL_USDT\""), std::string::npos);
  // Must NOT contain interleaved depth_level ("20") or interval ("0") strings
  EXPECT_EQ(payload.find("\"20\""), std::string::npos);
  EXPECT_EQ(payload.find("\"0\""), std::string::npos);
  // ETH_USDT must appear before SOL_USDT (stable order)
  EXPECT_LT(payload.find("ETH_USDT"), payload.find("SOL_USDT"));
}

TEST(ExchangeChannelTest, OrderBookSubscribePayloadSkipsDisabled) {
  std::vector<SymbolSpec> symbols = {
      {"ETH_USDT", false, 20, true},
      {"SOL_USDT", true, 20, true},
  };

  std::string payload = BuildGateioOrderBookSubscribePayload(symbols, 1779928915);

  EXPECT_EQ(payload.find("\"ETH_USDT\""), std::string::npos);
  EXPECT_NE(payload.find("\"SOL_USDT\""), std::string::npos);
}

}  // namespace
}  // namespace sqc
