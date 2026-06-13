#include "telemetry_agent.h"

#include <memory>
#include <thread>

#include "common/cpu_affinity.h"
#include "common/logger_init.h"
#include "config/config_loader.h"
#include "prometheus_exposer.h"
#include "quill/LogMacros.h"

namespace sqc {

namespace {
std::unique_ptr<TelemetryAgent> g_instance;
}

TelemetryAgent& TelemetryAgent::Init() {
  g_instance.reset(new TelemetryAgent());
  return *g_instance;
}

TelemetryAgent& TelemetryAgent::Instance() { return *g_instance; }

TelemetryAgent::TelemetryAgent()
    : exposer_(std::make_unique<PrometheusExposer>()),
      report_interval_ms_(Config::Instance().telemetry.report_interval_ms) {}

void TelemetryAgent::RegisterSlot(TelemetrySlot* slot) { slots_.push_back(slot); }

void TelemetryAgent::PollAll() {
  for(uint32_t i = 0; i < static_cast<uint32_t>(slots_.size()); ++i) {
    TelemetrySlot snapshot;
    ReadTelemetrySlot(slots_[i], snapshot);
    exposer_->SetLatencyUs(i, static_cast<double>(snapshot.market_data_delay_ns) / 1000.0);
    exposer_->SetQueueDepth(i, static_cast<double>(snapshot.queue_depth));
  }
}

void TelemetryAgent::Start() {
  thread_ = std::thread([this]() {
    if(Config::Instance().global.cpu_affinity)
      PinToCore(Config::Instance().threading_matrix.telemetry_core);
    Run();
  });
}

void TelemetryAgent::Run() {
  running_ = true;
  LOG_INFO(GetLogger(), "TelemetryAgent started (interval={}ms)", report_interval_ms_);
  while(running_) {
    PollAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(report_interval_ms_));
  }
}

void TelemetryAgent::Stop() {
  running_ = false;
  if(thread_.joinable()) thread_.join();
  if(exposer_) exposer_->Stop();  // stop civetweb HTTP server before main() returns
}

}  // namespace sqc
