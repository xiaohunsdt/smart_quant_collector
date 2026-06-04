#include <chrono>
#include <string>

#include "exchange/binance/binance_perpetual.h"
#include "exchange/binance/binance_spot.h"
#include "exchange/exchange_adapter.h"
#include "exchange/gateio/gateio_perpetual.h"
#include "exchange/gateio/gateio_spot.h"

namespace sqc {
namespace {

// ---- Binance subscribe builders ----

static std::vector<SubscriptionGroup> BinanceSpotBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  std::string name(symbol);
  for(auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return {
      {"wss://stream.binance.com/ws?timeUnit=MICROSECOND",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0, EventType::TICK},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth)" + std::to_string(depth_level) + R"(@100ms"],"id":2})", 500, EventType::DEPTH},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700, EventType::BOOK_TICKER},
       }}};
}

static std::vector<SubscriptionGroup> BinancePerpetualBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  std::string name(symbol);
  for(auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  std::string dl = std::to_string(depth_level);
  // Perpetual: @bookTicker + @depth on /public, @aggTrade on /market.
  // Perpetual timestamps are in milliseconds (futures API does not support timeUnit=MICROSECOND).
  return {
      {"wss://fstream.binance.com/market/ws",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0, EventType::TICK},
       }},
      {"wss://fstream.binance.com/public/ws",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700, EventType::BOOK_TICKER},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth)" + dl + R"(@100ms"],"id":2})", 500, EventType::DEPTH},
       }},
  };
};

// ---- Gate.io subscribe builders ----

static std::vector<SubscriptionGroup> GateioSpotBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  std::string dl = std::to_string(depth_level);
  return {
      {"wss://api.gateio.ws/ws/v4/",
       {
           {R"({"time":)" + ts + R"(,"channel":"spot.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0, EventType::TICK},
           {R"({"time":)" + ts + R"(,"channel":"spot.order_book","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"" + dl +
                "\",\"100ms\"]}",
            500, EventType::DEPTH},
           {R"({"time":)" + ts + R"(,"channel":"spot.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700,
            EventType::BOOK_TICKER},
       }}};
}

static std::vector<SubscriptionGroup> GateioPerpetualBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  std::string dl = std::to_string(depth_level);
  return {{"wss://fx-ws.gateio.ws/v4/ws/usdt",
           {
               {R"({"time":)" + ts + R"(,"channel":"futures.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0,
                EventType::TICK},
               {R"({"time":)" + ts + R"(,"channel":"futures.order_book","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"" + dl +
                    "\",\"0\"]}",
                500, EventType::DEPTH},
               {R"({"time":)" + ts + R"(,"channel":"futures.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700,
                EventType::BOOK_TICKER},
           }}};
}

// ---- Adapter instances ----

const ExchangeAdapter kBinanceSpotAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.binance.com",
    .build_subscribes = BinanceSpotBuildSubscribes,
    .peek_event_type = binance::PeekEventType,
    .parse = binance_spot::Parse,
};

const ExchangeAdapter kBinancePerpetualAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "fapi.binance.com",
    .build_subscribes = BinancePerpetualBuildSubscribes,
    .peek_event_type = binance::PeekEventType,
    .parse = binance_perpetual::Parse,
};

const ExchangeAdapter kGateioSpotAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioSpotBuildSubscribes,
    .peek_event_type = gateio_spot::PeekEventType,
    .parse = gateio_spot::Parse,
};

const ExchangeAdapter kGateioPerpetualAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioPerpetualBuildSubscribes,
    .peek_event_type = gateio_perpetual::PeekEventType,
    .parse = gateio_perpetual::Parse,
};

}  // namespace

const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type) {
  if(exchange_name == "binance") return channel_type == ChannelType::Spot ? &kBinanceSpotAdapter : &kBinancePerpetualAdapter;
  if(exchange_name == "gateio") return channel_type == ChannelType::Spot ? &kGateioSpotAdapter : &kGateioPerpetualAdapter;
  // "rithmic" handled via rithmic::RithmicEngine in main.cpp (callback-driven, not WebSocket)
  return nullptr;
}

}  // namespace sqc
