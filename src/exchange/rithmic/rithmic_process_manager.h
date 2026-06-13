#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace sqc {

class PubWorker;

namespace rithmic {

class RithmicChannelMap;
class RithmicReceiver;
class ShmSetup;

// ============================================================================
// RithmicProcessManager — owns the entire Rithmic pipeline lifecycle.
//
// Usage from main.cpp:
//   RithmicProcessManager mgr(cfg, deps);
//   mgr.Setup();   // register channels, create shm, spawn child, start threads
//   ... (main loop) ...
//   mgr.Shutdown(); // poison pill, stop threads, kill child
// ============================================================================

class RithmicProcessManager {
 public:
  struct Config {
    std::string shm_name = "/sqc_rithmic";
    std::string config_path = "config/config.yaml";
    uint32_t engine_core = 8;
    uint32_t forwarder_core = 9;
    uint32_t heartbeat_timeout_sec = 10;
    uint32_t max_restarts_per_min = 10;
    bool cpu_affinity = true;
  };

  struct Dependencies {
    PubWorker& pub_worker;
  };

  RithmicProcessManager(Config config, Dependencies deps);
  ~RithmicProcessManager();

  RithmicProcessManager(const RithmicProcessManager&) = delete;
  RithmicProcessManager& operator=(const RithmicProcessManager&) = delete;

  /// Phase 1+2: register channels, create shm, spawn child, start threads.
  /// Returns true if at least one Rithmic exchange was configured and started.
  bool Setup();

  /// Graceful shutdown: poison pill → receiver exit → SIGTERM child → join threads.
  void Shutdown();

  /// Health check (called by internal monitor thread every 500ms).
  bool CheckHealth();

 private:
  void RegisterChannels();
  bool IsChildAlive() const;
  bool SpawnChild();
  void StartThreads();

  Config config_;
  Dependencies deps_;

  // Owned pipeline
  std::unique_ptr<RithmicChannelMap> channel_map_;
  std::shared_ptr<ShmSetup> shm_;
  std::unique_ptr<RithmicReceiver> receiver_;

  // Child process
  pid_t child_pid_ = -1;
  std::chrono::steady_clock::time_point last_restart_;
  uint32_t restart_count_ = 0;

  // Threads
  std::thread receiver_thread_;
  std::thread monitor_thread_;
  std::atomic<bool> monitor_running_{false};
};

}  // namespace rithmic
}  // namespace sqc
