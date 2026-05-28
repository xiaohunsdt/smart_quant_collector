#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace sqc {

class GatewayRouter;

// Gateway supervisor with heartbeat monitoring, per spec §5.2
// Restarts gateway within 100ms on crash.
class GatewaySupervisor {
 public:
  GatewaySupervisor(const std::string& bind_addr);

  GatewaySupervisor(const GatewaySupervisor&) = delete;
  GatewaySupervisor& operator=(const GatewaySupervisor&) = delete;

  ~GatewaySupervisor();

  void Start();
  void Stop();

 private:
  void MonitorLoop();

  std::string bind_addr_;
  std::unique_ptr<GatewayRouter> router_;
  std::atomic<bool> running_{false};
  std::thread monitor_thread_;
};

}  // namespace sqc
