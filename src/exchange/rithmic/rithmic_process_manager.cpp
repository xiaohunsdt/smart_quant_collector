#include "src/exchange/rithmic/rithmic_process_manager.h"

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
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
#include "storage/storage_router.h"

#ifndef RITHMIC_GATEWAY_EXE
#define RITHMIC_GATEWAY_EXE "./rithmic_gateway"
#endif

extern char** environ;

namespace sqc {
namespace rithmic {

// ============================================================================
// Construction / Destruction
// ============================================================================

RithmicProcessManager::RithmicProcessManager(Config config)
    : config_(std::move(config)), last_restart_(std::chrono::steady_clock::now()) {}

RithmicProcessManager::~RithmicProcessManager() {
  // Defensive cleanup if Shutdown() wasn't called. Shutdown() is the normal
  // path; this only fires on early destruction. Threads MUST be joined first —
  // a joinable std::thread destructor calls std::terminate, and the monitor/
  // receiver threads reference members that are about to be destroyed.
  monitor_running_.store(false, std::memory_order_release);
  if(receiver_) receiver_->Stop();
  if(receiver_thread_.joinable()) receiver_thread_.join();
  if(monitor_thread_.joinable()) monitor_thread_.join();

  // Now that no thread touches child_pid_, kill + reap the child under the mutex.
  std::lock_guard<std::mutex> lk(child_mtx_);
  if(child_pid_ > 0) {
    ::kill(child_pid_, SIGKILL);
    ::waitpid(child_pid_, nullptr, 0);
    child_pid_ = -1;
  }
}

// ============================================================================
// RegisterChannels — Phase 1: collect all futures symbols from config
// ============================================================================

void RithmicProcessManager::RegisterChannels() {
  channel_map_ = std::make_unique<RithmicChannelMap>();

  try {
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
          // Register() throws on a genuine channel_id collision (two channels
          // hashing to the same 32-bit id). Catch here so a misconfiguration
          // surfaces as a clean startup failure rather than std::terminate.
          uint32_t id = ChannelRegistry::Instance().Register(info);
          channel_map_->Register(ex.name, sym.name, id, sym.enabled);
          StorageRouter::Instance().RegisterChannel(id, info, sym.persist_to_disk);
        }
      }
    }
  } catch(const std::exception& e) {
    LOG_CRITICAL(GetLogger(), "Rithmic: channel registration failed: {}", e.what());
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

  // The child (rithmic_gateway) must NOT inherit the parent's file descriptors
  // — it would otherwise hold the parent's network sockets, DolphinDB sockets,
  // ZMQ sockets and unrelated SHM fds open, which is both a descriptor leak and
  // a correctness hazard (e.g. open FDs keep listeners/timers alive). glibc's
  // POSIX_SPAWN_CLOEXEC_DEFAULT closes every inherited fd except 0/1/2. Fall
  // back to nothing on libc that lacks it (the gateway reopens everything it
  // needs from its own config).
  short flags = 0;
#ifdef POSIX_SPAWN_CLOEXEC_DEFAULT
  flags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
#endif
  // The parent blocks SIGINT/SIGTERM (SignalHandler::Install). Don't let the
  // child inherit that mask — give it an empty mask so its own handlers run.
  flags |= POSIX_SPAWN_SETSIGMASK;
  sigset_t empty_mask;
  sigemptyset(&empty_mask);
  posix_spawnattr_setsigmask(&attr, &empty_mask);
  posix_spawnattr_setflags(&attr, flags);

  int ret = posix_spawn(&child_pid_, exe.c_str(), &actions, &attr, const_cast<char* const*>(argv.data()), environ);

  posix_spawn_file_actions_destroy(&actions);
  posix_spawnattr_destroy(&attr);

  if(ret != 0 || child_pid_ <= 0) {
    LOG_ERROR(GetLogger(), "Rithmic: posix_spawn failed: {}", std::strerror(ret));
    // child_pid_ is only written by SpawnChild; callers either run single-
    // threaded (Setup, before threads start) or already hold child_mtx_
    // (CheckHealth's restart path), so no extra lock is taken here.
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

  // Stop child process. Hold child_mtx_ so we don't race CheckHealth's kill/reap.
  {
    std::lock_guard<std::mutex> lk(child_mtx_);
    if(child_pid_ > 0) {
      LOG_INFO(GetLogger(), "Rithmic: stopping child pid={}", child_pid_);
      ::kill(child_pid_, SIGTERM);

      int status;
      for(int i = 0; i < 50; ++i) {
        pid_t ret = ::waitpid(child_pid_, &status, WNOHANG);
        if(ret == child_pid_) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      // kill(pid, 0) is a side-effect-free liveness probe (no reap), so it's
      // safe here even if the pid was about to be recycled.
      if(child_pid_ > 0 && ::kill(child_pid_, 0) == 0) {
        LOG_WARNING(GetLogger(), "Rithmic: child did not exit, sending SIGKILL");
        ::kill(child_pid_, SIGKILL);
        ::waitpid(child_pid_, nullptr, 0);
      }
      child_pid_ = -1;
    }
  }

  if(monitor_thread_.joinable()) monitor_thread_.join();
}

// ============================================================================
// Health check + restart
// ============================================================================

bool RithmicProcessManager::CheckHealth() {
  // Liveness probe: waitpid(WNOHANG) both detects AND reaps a zombie child in
  // one step. Unlike kill(pid,0) (which returns 0 for zombies because they
  // still occupy the process table), this gives immediate detection of a
  // crashed-but-unreaped child so we restart without waiting for the heartbeat
  // timeout (~10s of zero production). All child_pid_ access is under child_mtx_.
  bool alive = false;
  {
    std::lock_guard<std::mutex> lk(child_mtx_);
    if(child_pid_ > 0) {
      int status;
      pid_t result = ::waitpid(child_pid_, &status, WNOHANG);
      if(result == child_pid_) {
        // Child exited — zombie reaped. Fall through to restart.
        child_pid_ = -1;
        alive = false;
      } else if(result == 0) {
        alive = true;  // still running
      } else {
        // result < 0 (ECHILD/etc.) — treat as dead and reset.
        child_pid_ = -1;
        alive = false;
      }
    }
  }

  if(alive) {
    auto* hdr = shm_->header();
    auto now_ns =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    uint64_t last_hb = hdr->heartbeat_ns.load(std::memory_order_acquire);

    if(last_hb > 0) {
      uint64_t elapsed_sec = (now_ns - last_hb) / 1'000'000'000ULL;
      if(elapsed_sec > config_.heartbeat_timeout_sec) {
        LOG_WARNING(GetLogger(), "Rithmic heartbeat timeout ({}s), killing child", elapsed_sec);
        std::lock_guard<std::mutex> lk(child_mtx_);
        if(child_pid_ > 0) {
          ::kill(child_pid_, SIGKILL);
          ::waitpid(child_pid_, nullptr, 0);
          child_pid_ = -1;
        }
        // Child killed on heartbeat timeout — fall through to immediate restart
        // instead of returning true (which deferred restart to the next cycle).
        alive = false;
      }
    }
    if(alive) return true;
  }

  // Child is dead — restart with rate limiting
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_restart_).count();

  if(elapsed < 60) {
    restart_count_++;
    if(restart_count_ > config_.max_restarts_per_min) {
      LOG_CRITICAL(GetLogger(), "Rithmic restart limit exceeded ({}/min), giving up", config_.max_restarts_per_min);
      // Stop the monitor loop so we don't keep re-logging and re-checking.
      monitor_running_.store(false, std::memory_order_release);
      return false;
    }
  } else {
    restart_count_ = 0;
  }
  last_restart_ = now;

  LOG_WARNING(GetLogger(), "Rithmic child not running, restarting (attempt {})", restart_count_);

  {
    // SpawnChild writes child_pid_ via posix_spawn; hold child_mtx_ so Shutdown
    // can't observe or signal a half-initialized pid, and reap any zombie first.
    std::lock_guard<std::mutex> lk(child_mtx_);
    if(child_pid_ > 0) {
      ::waitpid(child_pid_, nullptr, WNOHANG);
      child_pid_ = -1;
    }
    return SpawnChild();
  }
}

}  // namespace rithmic
}  // namespace sqc
