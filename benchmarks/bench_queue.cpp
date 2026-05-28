#include <benchmark/benchmark.h>
#include <thread>
#include "src/exchange/shard_queue.h"

namespace sqc {

static void BM_Queue_SPSC_1K(benchmark::State& state) {
  ShardQueue q(4096);
  std::thread consumer([&]() {
    int count = 0;
    while (count < state.max_iterations) {
      RawMessage msg = q.PopBlocking();
      if (msg.size == 0) break;
      ++count;
    }
  });

  for (auto _ : state) {
    RawMessage msg;
    msg.allocate(64);
    msg.size = 64;
    q.Push(std::move(msg));
  }

  q.PushPoisonPill();
  consumer.join();
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Queue_SPSC_1K)->Iterations(100000);

static void BM_Queue_TryPushPop(benchmark::State& state) {
  ShardQueue q(4096);
  for (auto _ : state) {
    RawMessage msg;
    msg.allocate(64);
    msg.size = 64;
    q.TryPush(std::move(msg));
    RawMessage out;
    q.TryPop(out);
    benchmark::DoNotOptimize(out);
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Queue_TryPushPop);

}  // namespace sqc

BENCHMARK_MAIN();
