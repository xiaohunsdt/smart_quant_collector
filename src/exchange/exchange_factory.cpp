#include "exchange/exchange_adapter.h"

#include <chrono>
#include <string>

#include "exchange/binance/binance_common.h"
#include "exchange/binance/binance_spot.h"
#include "exchange/binance/binance_perpetual.h"
#include "exchange/gateio/gateio_common.h"
#include "exchange/gateio/gateio_spot.h"
#include "exchange/gateio/gateio_perpetual.h"

namespace sqc {
namespace {

// ---- Binance subscribe builders ----

static std::vector<SubscriptionGroup> BinanceSpotBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  std::string name(symbol);
  for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return {{"wss://stream.binance.com/ws", {
    {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0},
    {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth@100ms"],"id":2})", 500},
    {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700},
  }}};
}

static std::vector<SubscriptionGroup> BinancePerpetualBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  std::string name(symbol);
  for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return {
    {"wss://fstream.binance.com/market/ws", {
      {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0},
    }},
    {"wss://fstream.binance.com/public/ws", {
      {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth@100ms"],"id":2})", 500},
      {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700},
    }},
  };
}

// ---- Gate.io subscribe builders ----

static std::vector<SubscriptionGroup> GateioSpotBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  return {{"wss://api.gateio.ws/ws/v4/", {
    {R"({"time":)" + ts + R"(,"channel":"spot.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0},
    {R"({"time":)" + ts + R"(,"channel":"spot.order_book_update","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"100ms\"]}", 500},
    {R"({"time":)" + ts + R"(,"channel":"spot.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700},
  }}};
}

static std::vector<SubscriptionGroup> GateioPerpetualBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  return {{"wss://fx-ws.gateio.ws/v4/ws/usdt", {
    {R"({"time":)" + ts + R"(,"channel":"futures.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0},
    {R"({"time":)" + ts + R"(,"channel":"futures.order_book_update","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"100ms\"]}", 500},
    {R"({"time":)" + ts + R"(,"channel":"futures.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700},
  }}};
}

// ---- Adapter instances ----

const ExchangeAdapter kBinanceSpotAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.binance.com",
    .build_subscribes = BinanceSpotBuildSubscribes,
    .parse = binance_spot::ParseMessage,
    .fetch_snapshot = binance_spot::FetchSnapshot,
};

const ExchangeAdapter kBinancePerpetualAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "fapi.binance.com",
    .build_subscribes = BinancePerpetualBuildSubscribes,
    .parse = binance_perpetual::ParseMessage,
    .fetch_snapshot = binance_perpetual::FetchSnapshot,
};

const ExchangeAdapter kGateioSpotAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioSpotBuildSubscribes,
    .parse = gateio_spot::ParseMessage,
    .fetch_snapshot = gateio_spot::FetchSnapshot,
};

const ExchangeAdapter kGateioPerpetualAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioPerpetualBuildSubscribes,
    .parse = gateio_perpetual::ParseMessage,
    .fetch_snapshot = gateio_perpetual::FetchSnapshot,
};

}  // namespace

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type) {
  if (exchange_name == "binance")
    return channel_type == ChannelType::Spot ? &kBinanceSpotAdapter : &kBinancePerpetualAdapter;
  if (exchange_name == "gateio")
    return channel_type == ChannelType::Spot ? &kGateioSpotAdapter : &kGateioPerpetualAdapter;
  return nullptr;
}

}  // namespace sqc
