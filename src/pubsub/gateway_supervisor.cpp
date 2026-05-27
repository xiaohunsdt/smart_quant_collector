#include "gateway_supervisor.h"

#include "gateway_router.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

GatewaySupervisor::GatewaySupervisor(const std::string& bind_addr)
    : bind_addr_(bind_addr) {}

void GatewaySupervisor::Start() {
  running_ = true;
  router_ = std::make_unique<GatewayRouter>(bind_addr_);
  monitor_thread_ = std::thread(&GatewaySupervisor::MonitorLoop, this);
  LOG_INFO(GetLogger(), "GatewaySupervisor started");
}

void GatewaySupervisor::Stop() {
  running_ = false;
  if (monitor_thread_.joinable()) {
    monitor_thread_.join();
  }
  router_.reset();
  LOG_INFO(GetLogger(), "GatewaySupervisor stopped");
}

void GatewaySupervisor::MonitorLoop() {
  while (running_) {
    // In production: real heartbeat check via ZMQ PING/PONG
    // For now: sleep and check running flag
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!running_) break;

    // If gateway crashed, restart within 100ms
    if (!router_) {
      LOG_WARNING(GetLogger(), "GatewaySupervisor: restarting gateway router");
      router_ = std::make_unique<GatewayRouter>(bind_addr_);
    }
  }
}

}  // namespace sqc
