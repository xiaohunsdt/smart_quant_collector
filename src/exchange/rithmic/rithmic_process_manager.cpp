#include "src/exchange/rithmic/rithmic_process_manager.h"

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <thread>

#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "src/common/cpu_affinity.h"
#include "src/common/logger_init.h"
#include "src/common/tick_data.h"
#include "src/config/config_loader.h"
#include "src/exchange/channel_mapping.h"
#include "src/exchange/data_dispatcher.h"
#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/exchange/rithmic/rithmic_receiver.h"
#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/exchange/rithmic/rithmic_types.h"
#include "src/orderbook/orderbook_event.h"
#include "src/storage/storage_router.h"

#ifndef RITHMIC_GATEWAY_EXE
#define RITHMIC_GATEWAY_EXE "./rithmic_gateway"
#endif

extern char** environ;

namespace sqc {
namespace rithmic {

// ============================================================================
// Construction / Destruction
// ============================================================================

RithmicProcessManager::RithmicProcessManager(Config config) : config_(std::move(config)) {}

RithmicProcessManager::~RithmicProcessManager() {
  if(child_pid_ > 0) {
    ::kill(child_pid_, SIGKILL);
    ::waitpid(child_pid_, nullptr, 0);
  }
}

// ============================================================================
// RegisterChannels — Phase 1: collect all futures symbols from config
// ============================================================================

void RithmicProcessManager::RegisterChannels() {
  channel_map_ = std::make_unique<RithmicChannelMap>();

  for(const auto& ex : sqc::Config::Instance().exchanges) {
    if(!ex.enabled) continue;
    bool is_rithmic = false;
    for(const auto& ch : ex.channels) {
      if(ch.type == "futures") {
        is_rithmic = true;
        break;
      }
    }
    if(!is_rithmic) continue;

    for(const auto& ch : ex.channels) {
      if(ch.type != "futures") continue;
      for(const auto& sym : ch.symbols) {
        if(!sym.enabled) continue;
        ChannelInfo info;
        info.exchange = ex.name;
        info.type = ChannelType::Futures;
        info.symbol = sym.name;
        info.depth_level = sym.depth_level;
        uint32_t id = ChannelRegistry::Instance().Register(info);
        channel_map_->Register(ex.name, sym.name, id, sym.enabled);
        StorageRouter::Instance().RegisterChannel(id, info, sym.persist_to_disk);
      }
    }
  }
}

// ============================================================================
// SpawnChild
// ============================================================================

bool RithmicProcessManager::SpawnChild() {
  std::string exe = RITHMIC_GATEWAY_EXE;

  std::vector<const char*> argv = {exe.c_str(), config_.config_path.c_str(), nullptr};

  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);

  posix_spawnattr_t attr;
  posix_spawnattr_init(&attr);

  int ret = posix_spawn(&child_pid_, exe.c_str(), &actions, &attr, const_cast<char* const*>(argv.data()), environ);

  posix_spawn_file_actions_destroy(&actions);
  posix_spawnattr_destroy(&attr);

  if(ret != 0 || child_pid_ <= 0) {
    LOG_ERROR(GetLogger(), "Rithmic: posix_spawn failed: {}", std::strerror(ret));
    child_pid_ = -1;
    return false;
  }

  LOG_INFO(GetLogger(), "Rithmic child started: pid={}", child_pid_);
  return true;
}

// ============================================================================
// Setup — Phase 1+2: register, freeze, create shm, spawn, start threads
// ============================================================================

bool RithmicProcessManager::Setup() {
  RegisterChannels();
  if(channel_map_->Subscriptions().empty()) {
    return false;
  }

  constexpr size_t kShmSize = shm_layout::kTotalShmSize;
  try {
    shm_ = std::make_shared<ShmSetup>(config_.shm_name.c_str(), kShmSize, true);
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "Rithmic: shm creation failed: {}", e.what());
    return false;
  }

  // Placement-construct the 3 MPSC queues in shared memory. ShmSetup only
  // construct-initializes ShmHeader; the queues following it must be
  // initialized explicitly.
  {
    auto* base = static_cast<char*>(shm_->addr());
    new (base + shm_layout::kTickQueueOffset) shm_layout::TickQueue();
    new (base + shm_layout::kDepthQueueOffset) shm_layout::DepthQueue();
    new (base + shm_layout::kBookTickerQueueOffset) shm_layout::BookTickerQueue();
  }

  if(!SpawnChild()) {
    shm_.reset();
    return false;
  }

  // Create receiver — handlers share the same DataDispatcher as crypto parsers.
  // Telemetry slot is null: Rithmic uses a dedicated forwarder thread without
  // a per-shard seqlock slot; latency is monitored via SHM heartbeat instead.
  DataDispatcher dispatcher{nullptr, nullptr, 0};
  receiver_ = std::make_unique<RithmicReceiver>(
      shm_->tick_queue(), shm_->depth_queue(), shm_->book_ticker_queue(), config_.forwarder_core,
      [dispatcher](TickData tick) { dispatcher.OnTick(std::move(tick)); },
      [dispatcher](uint32_t cid, const DepthUpdateEvent& ev) { dispatcher.OnDepth(cid, ev); },
      [dispatcher](uint32_t cid, BookTickerEvent ev) { dispatcher.OnBookTicker(cid, std::move(ev)); });

  StartThreads();
  return true;
}

// ============================================================================
// StartThreads
// ============================================================================

void RithmicProcessManager::StartThreads() {
  monitor_running_.store(true, std::memory_order_release);

  receiver_thread_ = std::thread([this]() {
    if(config_.cpu_affinity) PinToCore(receiver_->core_id());
    receiver_->Run();
  });

  monitor_thread_ = std::thread([this]() {
    while(monitor_running_.load(std::memory_order_acquire)) {
      CheckHealth();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });
}

// ============================================================================
// Shutdown
// ============================================================================

void RithmicProcessManager::Shutdown() {
  // Stop monitor first
  monitor_running_.store(false, std::memory_order_release);

  // Stop receiver — running_ flag causes TryPop loop to exit within microseconds
  if(receiver_) receiver_->Stop();
  if(receiver_thread_.joinable()) receiver_thread_.join();

  // Stop child process
  if(child_pid_ > 0) {
    LOG_INFO(GetLogger(), "Rithmic: stopping child pid={}", child_pid_);
    ::kill(child_pid_, SIGTERM);

    int status;
    for(int i = 0; i < 50; ++i) {
      pid_t ret = ::waitpid(child_pid_, &status, WNOHANG);
      if(ret == child_pid_) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if(IsChildAlive()) {
      LOG_WARNING(GetLogger(), "Rithmic: child did not exit, sending SIGKILL");
      ::kill(child_pid_, SIGKILL);
      ::waitpid(child_pid_, nullptr, 0);
    }
    child_pid_ = -1;
  }

  if(monitor_thread_.joinable()) monitor_thread_.join();
}

// ============================================================================
// Health check + restart
// ============================================================================

bool RithmicProcessManager::IsChildAlive() const {
  if(child_pid_ <= 0) return false;
  int status;
  return ::waitpid(child_pid_, &status, WNOHANG) == 0;
}

bool RithmicProcessManager::CheckHealth() {
  if(IsChildAlive()) {
    auto* hdr = shm_->header();
    auto now_ns =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    uint64_t last_hb = hdr->heartbeat_ns.load(std::memory_order_acquire);

    if(last_hb > 0) {
      uint64_t elapsed_sec = (now_ns - last_hb) / 1'000'000'000ULL;
      if(elapsed_sec > config_.heartbeat_timeout_sec) {
        LOG_WARNING(GetLogger(), "Rithmic heartbeat timeout ({}s), killing child", elapsed_sec);
        ::kill(child_pid_, SIGKILL);
        ::waitpid(child_pid_, nullptr, 0);
        child_pid_ = -1;
      } else {
        return true;
      }
    }
    return true;
  }

  // Child is dead — restart with rate limiting
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_restart_).count();

  if(elapsed < 60) {
    restart_count_++;
    if(restart_count_ > config_.max_restarts_per_min) {
      LOG_CRITICAL(GetLogger(), "Rithmic restart limit exceeded ({}/min), giving up", config_.max_restarts_per_min);
      return false;
    }
  } else {
    restart_count_ = 0;
  }
  last_restart_ = now;

  LOG_WARNING(GetLogger(), "Rithmic child not running, restarting (attempt {})", restart_count_);

  if(child_pid_ > 0) {
    ::kill(child_pid_, SIGKILL);
    ::waitpid(child_pid_, nullptr, 0);
    child_pid_ = -1;
  }

  return SpawnChild();
}

}  // namespace rithmic
}  // namespace sqc
