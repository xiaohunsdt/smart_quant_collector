#include "exchange/exchange_adapter.h"

#include <chrono>
#include <string>

#include "exchange/binance/binance_parser.h"
#include "exchange/binance/binance_snapshot_client.h"
#include "exchange/gateio/gateio_parser.h"
#include "exchange/gateio/gateio_snapshot_client.h"

namespace sqc {
namespace {

// ---- Binance subscribe builder ----

std::vector<std::pair<std::string, uint32_t>> BinanceBuildSubscribes(ChannelType /*channel_type*/, std::string_view symbol, uint32_t /*depth_level*/) {
  std::string name(symbol);
  for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

  std::vector<std::pair<std::string, uint32_t>> subs;
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0);
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth@100ms"],"id":2})", 500);
  subs.emplace_back(R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700);
  return subs;
}

// ---- Gate.io subscribe builder ----

std::vector<std::pair<std::string, uint32_t>> GateioBuildSubscribes(ChannelType channel_type, std::string_view symbol, uint32_t /*depth_level*/) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);

  std::string prefix = channel_type == ChannelType::Spot ? "spot" : "futures";

  std::vector<std::pair<std::string, uint32_t>> subs;
  // 1. trades (immediate)
  subs.emplace_back(
      R"({"time":)" + ts +
          R"(,"channel":")" + prefix + R"(.trades","event":"subscribe","payload":[")" +
          std::string(symbol) + R"("]})",
      0);
  // 2. incremental order_book_update with 100ms interval (delayed 500ms)
  subs.emplace_back(
      R"({"time":)" + ts +
          R"(,"channel":")" + prefix + R"(.order_book_update","event":"subscribe","payload":[")" +
          std::string(symbol) + "\",\"100ms\"]}",
      500);
  // 3. book_ticker for best bid/ask (delayed 700ms)
  subs.emplace_back(
      R"({"time":)" + ts +
          R"(,"channel":")" + prefix + R"(.book_ticker","event":"subscribe","payload":[")" +
          std::string(symbol) + R"("]})",
      700);
  return subs;
}

// ---- fetch_snapshot wrappers (match unified 3-arg signature) ----

OrderbookSnapshot BinanceFetchSnapshot(std::string_view rest_host,
                                       ChannelType /*channel_type*/,
                                       std::string_view symbol) {
  return binance::FetchSnapshot(rest_host, symbol);
}

OrderbookSnapshot GateioFetchSnapshot(std::string_view rest_host,
                                      ChannelType channel_type,
                                      std::string_view symbol) {
  return gateio::FetchSnapshot(rest_host, channel_type, symbol);
}

// ---- Adapter instances ----

const ExchangeAdapter kBinanceAdapter = {
    .name = "binance",
    .snapshot_mode = false,
    .spot = {"wss://stream.binance.com:9443/ws", "api.binance.com"},
    .perpetual = {"wss://fstream.binance.com/ws", "fapi.binance.com"},
    .build_subscribes = BinanceBuildSubscribes,
    .parse = binance_parser::ParseMessage,
    .fetch_snapshot = BinanceFetchSnapshot,
};

const ExchangeAdapter kGateioAdapter = {
    .name = "gateio",
    .snapshot_mode = false,
    .spot = {"wss://api.gateio.ws/ws/v4/", "api.gateio.ws"},
    .perpetual = {"wss://fx-ws.gateio.ws/v4/ws/usdt", "api.gateio.ws"},
    .build_subscribes = GateioBuildSubscribes,
    .parse = gateio_parser::ParseMessage,
    .fetch_snapshot = GateioFetchSnapshot,
};

}  // namespace

const ExchangeAdapter* GetAdapter(std::string_view exchange_name) {
  if (exchange_name == "binance") return &kBinanceAdapter;
  if (exchange_name == "gateio") return &kGateioAdapter;
  return nullptr;
}

}  // namespace sqc
