#include <benchmark/benchmark.h>
#include <cstring>
#include "simdjson.h"
#include "src/common/tick_data.h"
#include "src/exchange/binance/binance_spot.h"
#include "src/exchange/gateio/gateio_spot.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

// Real Binance trade JSON
static constexpr const char* kBinanceTradeJson = R"({
  "e":"trade","E":1715600000000,"s":"BTCUSDT","t":123456789,
  "p":"65000.50","q":"0.12345678","b":987654321,"a":987654322,
  "T":1715600000000,"m":false
})";

// Real Gate.io futures.tickers JSON
static constexpr const char* kGateioTickerJson = R"({
  "time":1779916108,"time_ms":1779916108315,"channel":"futures.tickers",
  "event":"update","result":[{"contract":"BTC_USDT","last":"75180.8",
  "change_percentage":"-1.0246","total_size":"616999728",
  "volume_24h":"562890187","volume_24h_base":"56289.0187",
  "volume_24h_quote":"4231853464","volume_24h_settle":"4231853464",
  "mark_price":"75180.8","funding_rate":"0.000094",
  "funding_rate_indicative":"0.000094","index_price":"75210.0",
  "quanto_base_rate":"","low_24h":"74635.0","high_24h":"76143.0",
  "price_type":"last","change_from":"24h","change_price":"-778.3",
  "t":1779916107555}]
})";

static void BM_BinanceParseTrade(benchmark::State& state) {
  size_t len = std::strlen(kBinanceTradeJson);
  std::string padded(kBinanceTradeJson);
  padded.resize(len + simdjson::SIMDJSON_PADDING, '\0');
  simdjson::ondemand::parser parser;

  for (auto _ : state) {
    simdjson::ondemand::document doc;
    auto err = parser.iterate(padded.data(), len, padded.size()).get(doc);
    if (err) state.SkipWithError("iterate failed");
    auto result = binance_spot::ParseMessage(doc, 1);
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BinanceParseTrade);

static void BM_GateioParseTicker(benchmark::State& state) {
  size_t len = std::strlen(kGateioTickerJson);
  std::string padded(kGateioTickerJson);
  padded.resize(len + simdjson::SIMDJSON_PADDING, '\0');
  simdjson::ondemand::parser parser;

  for (auto _ : state) {
    simdjson::ondemand::document doc;
    auto err = parser.iterate(padded.data(), len, padded.size()).get(doc);
    if (err) state.SkipWithError("iterate failed");
    auto result = gateio_spot::ParseMessage(doc, 2);
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_GateioParseTicker);

}  // namespace sqc

BENCHMARK_MAIN();
