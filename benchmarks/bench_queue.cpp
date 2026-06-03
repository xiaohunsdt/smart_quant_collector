#include <benchmark/benchmark.h>

#include <thread>

#include "src/common/signal_handler.h"
#include "src/exchange/shard_queue.h"

namespace sqc {

static void BM_Queue_SPSC_1K(benchmark::State& state) {
  ShardQueue q(4096);
  // Capture max_iterations *before* spawning the consumer thread —
  // Google Benchmark's State is not thread-safe, and reading
  // state.max_iterations from a secondary thread is a data race.
  const auto total = state.max_iterations;
  std::thread consumer([&]() {
    int count = 0;
    while(count < total) {
      RawMessage msg;
      while(!q.TryPop(msg)) {
        if(SignalHandler::IsShutdownRequested()) return;
        std::this_thread::yield();
      }
      if(msg.size == 0) break;
      ++count;
    }
  });

  for(auto _ : state) {
    RawMessage msg;
    (void)msg.allocate(64);
    msg.size = 64;
    q.TryPush(std::move(msg));
  }

  q.PushPoisonPill();
  consumer.join();
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Queue_SPSC_1K)->Iterations(100000);

static void BM_Queue_TryPushPop(benchmark::State& state) {
  ShardQueue q(4096);
  for(auto _ : state) {
    RawMessage msg;
    (void)msg.allocate(64);
    msg.size = 64;
    bool pushed = q.TryPush(std::move(msg));
    RawMessage out;
    bool popped = q.TryPop(out);
    benchmark::DoNotOptimize(out);
    (void)pushed;
    (void)popped;
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Queue_TryPushPop);

}  // namespace sqc

BENCHMARK_MAIN();
