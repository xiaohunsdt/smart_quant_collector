#include "prometheus_exposer.h"

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

struct PrometheusExposer::Metrics {
  prometheus::Family<prometheus::Gauge>* latency_family;
  prometheus::Family<prometheus::Gauge>* queue_depth_family;
  prometheus::Family<prometheus::Counter>* zmq_dropped_family;
};

PrometheusExposer::PrometheusExposer(uint16_t port)
    : registry_(std::make_shared<prometheus::Registry>()),
      metrics_(std::make_unique<Metrics>()) {
  metrics_->latency_family = &prometheus::BuildGauge()
                                  .Name("collector_latency_us")
                                  .Help("End-to-end latency in microseconds")
                                  .Register(*registry_);

  metrics_->queue_depth_family = &prometheus::BuildGauge()
                                      .Name("collector_queue_depth")
                                      .Help("Shard queue depth")
                                      .Register(*registry_);

  metrics_->zmq_dropped_family = &prometheus::BuildCounter()
                                      .Name("collector_zmq_dropped_total")
                                      .Help("Total ZMQ messages dropped")
                                      .Register(*registry_);

  try {
    exposer_ = std::make_unique<prometheus::Exposer>(
        std::string("0.0.0.0:") + std::to_string(port));
    exposer_->RegisterCollectable(registry_);
    healthy_ = true;
    LOG_INFO(GetLogger(), "PrometheusExposer listening on port {}", port);
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "PrometheusExposer failed: {}", e.what());
    healthy_ = false;
  }
}

PrometheusExposer::~PrometheusExposer() = default;

bool PrometheusExposer::IsHealthy() const { return healthy_; }

void PrometheusExposer::SetLatencyUs(uint32_t channel_id, double value) {
  std::lock_guard<std::mutex> lock(metrics_mtx_);
  auto it = latency_gauges_.find(channel_id);
  if (it == latency_gauges_.end()) {
    auto& g = metrics_->latency_family->Add({{"channel", std::to_string(channel_id)}});
    it = latency_gauges_.emplace(channel_id, &g).first;
  }
  it->second->Set(value);
}

void PrometheusExposer::SetQueueDepth(uint32_t channel_id, double value) {
  std::lock_guard<std::mutex> lock(metrics_mtx_);
  auto it = queue_depth_gauges_.find(channel_id);
  if (it == queue_depth_gauges_.end()) {
    auto& g = metrics_->queue_depth_family->Add({{"channel", std::to_string(channel_id)}});
    it = queue_depth_gauges_.emplace(channel_id, &g).first;
  }
  it->second->Set(value);
}

void PrometheusExposer::IncrementZmqDropped(uint32_t channel_id) {
  std::lock_guard<std::mutex> lock(metrics_mtx_);
  auto it = zmq_dropped_counters_.find(channel_id);
  if (it == zmq_dropped_counters_.end()) {
    auto& c = metrics_->zmq_dropped_family->Add({{"channel", std::to_string(channel_id)}});
    it = zmq_dropped_counters_.emplace(channel_id, &c).first;
  }
  it->second->Increment();
}

}  // namespace sqc
