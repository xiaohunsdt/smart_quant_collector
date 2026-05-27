#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "src/common/telemetry_slot.h"

namespace sqc {

class PrometheusExposer;

// Telemetry agent running on Core 7, per spec §7.2
// Polls POSIX shared memory slots and aggregates into Prometheus.
class TelemetryAgent {
 public:
  TelemetryAgent(PrometheusExposer* exposer, uint32_t report_interval_ms);

  void RegisterSlot(const std::string& name, TelemetrySlot* slot);
  void PollAll();
  void IncrementZmqDropped(uint32_t channel_id);

  // Run loop: polls slots periodically until stopped
  void Run();
  void Stop();

 private:
  struct SlotEntry {
    std::string name;
    TelemetrySlot* ptr;
  };

  PrometheusExposer* exposer_;
  uint32_t report_interval_ms_;
  std::vector<SlotEntry> slots_;
  uint64_t total_zmq_dropped_ = 0;
  bool running_ = false;
};

}  // namespace sqc
