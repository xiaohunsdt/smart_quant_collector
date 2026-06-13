#pragma once

#include <chrono>
#include <cstddef>
#include <cstring>

#include "src/common/telemetry_slot.h"
#include "src/common/tick_data.h"
#include "src/exchange/shard_queue.h"
#include "src/orderbook/orderbook_event.h"
#include "src/pubsub/pub_worker.h"

namespace sqc {

// Encapsulates the common "receive parsed event → route to storage + pub +
// telemetry" logic shared between the crypto parser path (ShardParserWorker)
// and the Rithmic receiver path (RithmicProcessManager).
//
// Construct one DataDispatcher per logical worker.  `telemetry_slot` and
// `shard_queue` may be nullptr when telemetry is not relevant (e.g. Rithmic
// forwarder thread).
// ChannelRegistry and StorageRouter are accessed via their singletons.
struct DataDispatcher {
  PubWorker&        pub_worker;
  TelemetrySlot*    telemetry_slot = nullptr;  // null → skip telemetry write
  const ShardQueue* shard_queue    = nullptr;  // null → queue depth reported as 0
  size_t            shard_idx      = 0;

  void OnTick(TickData tick) const noexcept;
  void OnDepth(uint32_t channel_id, const DepthUpdateEvent& event) const noexcept;
  void OnBookTicker(uint32_t channel_id, BookTickerEvent event) const noexcept;

 private:
  static uint64_t NowNs() noexcept {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
  }

  void WriteTelemetry(uint64_t latency_ns) const noexcept {
    if (!telemetry_slot) return;
    const size_t q_depth = shard_queue ? shard_queue->size() : 0;
    WriteTelemetrySlot(telemetry_slot, latency_ns, q_depth, 0,
                       pub_worker.dropped_count());
  }
};

}  // namespace sqc
