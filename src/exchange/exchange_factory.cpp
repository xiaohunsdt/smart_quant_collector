#include "exchange/exchange_adapter.h"

#include <chrono>
#include <string>

#include "exchange/binance/binance_common.h"
#include "exchange/binance/binance_spot_parser.h"
#include "exchange/binance/binance_perpetual_parser.h"
#include "exchange/binance/binance_snapshot_client.h"
#include "exchange/gateio/gateio_common.h"
#include "exchange/gateio/gateio_spot_parser.h"
#include "exchange/gateio/gateio_perpetual_parser.h"
#include "exchange/gateio/gateio_snapshot_client.h"

namespace sqc {
namespace {

// ---- Binance subscribe builder (shared) ----

std::vector<std::pair<std::string, uint32_t>> BinanceBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  std::string name(symbol);
  for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

  std::vector<std::pair<std::string, uint32_t>> subs;
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0);
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth@100ms"],"id":2})", 500);
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700);
  return subs;
}

// ---- Gate.io subscribe builders (per channel type) ----

std::vector<std::pair<std::string, uint32_t>> GateioSpotBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);

  std::vector<std::pair<std::string, uint32_t>> subs;
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"spot.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0);
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"spot.order_book_update","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"100ms\"]}", 500);
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"spot.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700);
  return subs;
}

std::vector<std::pair<std::string, uint32_t>> GateioPerpetualBuildSubscribes(std::string_view symbol, uint32_t /*depth_level*/) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);

  std::vector<std::pair<std::string, uint32_t>> subs;
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"futures.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0);
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"futures.order_book_update","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"100ms\"]}", 500);
  subs.emplace_back(R"({"time":)" + ts + R"(,"channel":"futures.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700);
  return subs;
}

// ---- fetch_snapshot wrappers ----

OrderbookSnapshot BinanceSpotFetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  return binance::FetchSnapshot(rest_host, symbol);
}

OrderbookSnapshot BinancePerpetualFetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  return binance::FetchSnapshot(rest_host, symbol);
}

OrderbookSnapshot GateioSpotFetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  return gateio::FetchSnapshot(rest_host, ChannelType::Spot, symbol);
}

OrderbookSnapshot GateioPerpetualFetchSnapshot(std::string_view rest_host, std::string_view symbol) {
  return gateio::FetchSnapshot(rest_host, ChannelType::Perpetual, symbol);
}

// ---- Adapter instances ----

const ExchangeAdapter kBinanceSpotAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Spot,
    .ws_url = "wss://stream.binance.com:9443/ws",
    .rest_host = "api.binance.com",
    .build_subscribes = BinanceBuildSubscribes,
    .parse = binance_spot::ParseMessage,
    .fetch_snapshot = BinanceSpotFetchSnapshot,
};

const ExchangeAdapter kBinancePerpetualAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Perpetual,
    .ws_url = "wss://fstream.binance.com/ws",
    .rest_host = "fapi.binance.com",
    .build_subscribes = BinanceBuildSubscribes,
    .parse = binance_perpetual::ParseMessage,
    .fetch_snapshot = BinancePerpetualFetchSnapshot,
};

const ExchangeAdapter kGateioSpotAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Spot,
    .ws_url = "wss://api.gateio.ws/ws/v4/",
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioSpotBuildSubscribes,
    .parse = gateio_spot::ParseMessage,
    .fetch_snapshot = GateioSpotFetchSnapshot,
};

const ExchangeAdapter kGateioPerpetualAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Perpetual,
    .ws_url = "wss://fx-ws.gateio.ws/v4/ws/usdt",
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioPerpetualBuildSubscribes,
    .parse = gateio_perpetual::ParseMessage,
    .fetch_snapshot = GateioPerpetualFetchSnapshot,
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
