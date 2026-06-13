#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "src/common/telemetry_slot.h"

namespace sqc {

class PrometheusExposer;

// Telemetry agent running on Core 7, per spec §7.2
// Polls seqlock slots and aggregates into Prometheus.
// Singleton — call Init() once during startup, then Instance() from any thread.
//
// Lifecycle:
//   TelemetryAgent::Init()      — call once during single-threaded startup
//   TelemetryAgent::Instance()  — access singleton after Init()
//   RegisterSlot(slot)          — register parser slots before Start()
//   Start()                     — spawn dedicated thread, pin to telemetry_core
//   Stop()                      — signal loop to exit and join thread
class TelemetryAgent {
 public:
  static TelemetryAgent& Init();      // call once during single-threaded startup
  static TelemetryAgent& Instance();  // access from any thread after Init()

  TelemetryAgent(const TelemetryAgent&) = delete;
  TelemetryAgent& operator=(const TelemetryAgent&) = delete;

  void RegisterSlot(TelemetrySlot* slot);
  void PollAll();

  void Start();  // spawn dedicated thread (pins to telemetry_core) and begin polling
  void Stop();   // signal loop to exit and join thread

 private:
  TelemetryAgent();  // reads Config::Instance(); creates PrometheusExposer internally

  void Run();  // poll loop — called internally by Start()

  std::unique_ptr<PrometheusExposer> exposer_;
  uint32_t report_interval_ms_;
  std::vector<TelemetrySlot*> slots_;
  std::atomic<bool> running_{false};
  std::thread thread_;
};

}  // namespace sqc
