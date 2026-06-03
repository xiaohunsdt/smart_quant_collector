#pragma once

#include <atomic>
#include <cstdint>

namespace sqc {

// 64-byte aligned exclusive slot, per spec §7.2
struct alignas(64) TelemetrySlot {
  std::atomic<uint64_t> version{0};
  uint64_t market_data_delay_ns{0};
  uint64_t queue_depth{0};
  uint64_t sequence_gap_count{0};
  uint64_t zmq_dropped_count{0};
};

// Child-process writer (Core 3/4 high-frequency write)
inline void WriteTelemetrySlot(TelemetrySlot* slot, uint64_t delay_ns, uint64_t q_depth, uint64_t gap_count, uint64_t zmq_dropped) {
  slot->version.fetch_add(1, std::memory_order_relaxed);  // odd = writing
  slot->market_data_delay_ns = delay_ns;
  slot->queue_depth = q_depth;
  slot->sequence_gap_count = gap_count;
  slot->zmq_dropped_count = zmq_dropped;
  slot->version.fetch_add(1, std::memory_order_release);  // even = done
}

// Parent-process reader (Core 7 telemetry thread polling)
inline void ReadTelemetrySlot(TelemetrySlot* slot, TelemetrySlot& snapshot) {
  uint64_t v1, v2;
  do {
    v1 = slot->version.load(std::memory_order_acquire);
    snapshot.market_data_delay_ns = slot->market_data_delay_ns;
    snapshot.queue_depth = slot->queue_depth;
    snapshot.sequence_gap_count = slot->sequence_gap_count;
    snapshot.zmq_dropped_count = slot->zmq_dropped_count;
    v2 = slot->version.load(std::memory_order_acquire);
  } while((v1 & 1) || (v1 != v2));  // retry if odd or version changed during read
}

}  // namespace sqc
