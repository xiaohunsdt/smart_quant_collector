#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace prometheus {
class Registry;
class Exposer;
class Gauge;
class Counter;
}  // namespace prometheus

namespace sqc {

// Prometheus metrics exposer, per spec §7.2
class PrometheusExposer {
 public:
  explicit PrometheusExposer(uint16_t port);
  ~PrometheusExposer();

  PrometheusExposer(const PrometheusExposer&) = delete;
  PrometheusExposer& operator=(const PrometheusExposer&) = delete;

  bool IsHealthy() const;

  void SetLatencyUs(uint32_t channel_id, double value);
  void SetQueueDepth(uint32_t channel_id, double value);
  void IncrementZmqDropped(uint32_t channel_id);

 private:
  std::shared_ptr<prometheus::Registry> registry_;
  std::unique_ptr<prometheus::Exposer> exposer_;
  uint16_t port_;
  bool healthy_ = false;

  // Metric families (created dynamically per channel)
  struct Metrics;
  std::unique_ptr<Metrics> metrics_;
};

}  // namespace sqc
