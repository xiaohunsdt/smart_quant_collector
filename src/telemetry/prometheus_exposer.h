#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace prometheus {
class Registry;
class Exposer;
class Gauge;
class Counter;
}  // namespace prometheus

namespace sqc {

// Prometheus metrics exposer, per spec §7.2
// Metrics are lazily created and cached per channel_id to avoid
// leaking memory via Family::Add() on every update (F16).
class PrometheusExposer {
 public:
  PrometheusExposer();
  ~PrometheusExposer();

  PrometheusExposer(const PrometheusExposer&) = delete;
  PrometheusExposer& operator=(const PrometheusExposer&) = delete;

  bool IsHealthy() const;
  void Stop();  // explicitly stop the HTTP server; safe to call multiple times

  void SetLatencyUs(uint32_t channel_id, double value);
  void SetQueueDepth(uint32_t channel_id, double value);
  void IncrementZmqDropped(uint32_t channel_id);

 private:
  std::shared_ptr<prometheus::Registry> registry_;
  std::unique_ptr<prometheus::Exposer> exposer_;
  bool healthy_ = false;

  // Metric families
  struct Metrics;
  std::unique_ptr<Metrics> metrics_;

  // Cached metric instances per channel_id (protected by metrics_mtx_)
  std::mutex metrics_mtx_;
  std::unordered_map<uint32_t, prometheus::Gauge*> latency_gauges_;
  std::unordered_map<uint32_t, prometheus::Gauge*> queue_depth_gauges_;
  std::unordered_map<uint32_t, prometheus::Counter*> zmq_dropped_counters_;
};

}  // namespace sqc
