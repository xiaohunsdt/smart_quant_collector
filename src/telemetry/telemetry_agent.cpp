#include "telemetry_agent.h"

#include <thread>

#include "prometheus_exposer.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

TelemetryAgent::TelemetryAgent(PrometheusExposer* exposer, uint32_t report_interval_ms)
    : exposer_(exposer), report_interval_ms_(report_interval_ms) {}

void TelemetryAgent::RegisterSlot(const std::string& name, TelemetrySlot* slot) {
  slots_.push_back({name, slot});
}

void TelemetryAgent::PollAll() {
  for (uint32_t i = 0; i < static_cast<uint32_t>(slots_.size()); ++i) {
    auto& entry = slots_[i];
    TelemetrySlot snapshot;
    ReadTelemetrySlot(entry.ptr, snapshot);
    total_zmq_dropped_ += snapshot.zmq_dropped_count;

    if (exposer_) {
      exposer_->SetLatencyUs(i, static_cast<double>(snapshot.market_data_delay_ns) / 1000.0);
      exposer_->SetQueueDepth(i, static_cast<double>(snapshot.queue_depth));
    }
  }
}

void TelemetryAgent::IncrementZmqDropped(uint32_t channel_id) {
  if (channel_id < slots_.size()) {
    slots_[channel_id].ptr->zmq_dropped_count++;
  }
}

void TelemetryAgent::Run() {
  running_ = true;
  LOG_INFO(GetLogger(), "TelemetryAgent started (interval={}ms)",
           report_interval_ms_);
  while (running_) {
    PollAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(report_interval_ms_));
  }
}

void TelemetryAgent::Stop() { running_ = false; }

}  // namespace sqc
