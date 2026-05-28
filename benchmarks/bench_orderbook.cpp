#include <benchmark/benchmark.h>
#include "src/orderbook/local_lob.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

static void BM_LOB_UpdateDepth5(benchmark::State& state) {
  LocalLOB lob(10);

  // Initial snapshot
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  for (int i = 0; i < 5; ++i) {
    snap.bids[i] = {50000.0 - i * 10.0, 1.0 + i};
    snap.asks[i] = {50100.0 + i * 10.0, 1.0 + i};
  }
  snap.bid_count = 5;
  snap.ask_count = 5;
  lob.ApplySnapshot(snap);

  // Depth update: modify 3 bids + 3 asks
  DepthUpdateEvent event{};
  event.first_update_id = 2;
  event.last_update_id = 3;
  event.bids[0] = {50000.0, 2.5};
  event.bids[1] = {49990.0, 0.0};  // delete
  event.bids[2] = {49980.0, 5.0};
  event.bid_count = 3;
  event.asks[0] = {50100.0, 3.0};
  event.asks[1] = {50110.0, 0.0};  // delete
  event.asks[2] = {50120.0, 4.0};
  event.ask_count = 3;

  for (auto _ : state) {
    lob.ApplySnapshot(snap);  // reset state before each iteration
    lob.UpdateDepth(event);
    benchmark::DoNotOptimize(lob);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LOB_UpdateDepth5);

static void BM_LOB_TopBidsAsks(benchmark::State& state) {
  LocalLOB lob(10);
  OrderbookSnapshot snap{};
  snap.lastUpdateId = 1;
  for (int i = 0; i < 10; ++i) {
    snap.bids[i] = {50000.0 - i * 10.0, 1.0 + i};
    snap.asks[i] = {50100.0 + i * 10.0, 1.0 + i};
  }
  snap.bid_count = 10;
  snap.ask_count = 10;
  lob.ApplySnapshot(snap);

  PriceLevel bids[100];
  PriceLevel asks[100];

  for (auto _ : state) {
    uint32_t nb = lob.TopBids(bids, 10);
    uint32_t na = lob.TopAsks(asks, 10);
    benchmark::DoNotOptimize(bids);
    benchmark::DoNotOptimize(asks);
    benchmark::DoNotOptimize(nb);
    benchmark::DoNotOptimize(na);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_LOB_TopBidsAsks);

}  // namespace sqc

BENCHMARK_MAIN();
