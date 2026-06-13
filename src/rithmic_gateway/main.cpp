#include <csignal>
#include <cstdint>
#include <string>
#include <vector>

#include "quill/LogMacros.h"
#include "quill/SimpleSetup.h"

#ifdef WinOS
#include <windows.h>
#define sleep(s) Sleep((s) * 1000)
#define signal(s, h) signal(s, h)
#else
#include <unistd.h>
#endif

#include <pthread.h>
#include <sched.h>
#include <sys/prctl.h>

#include "rithmic_engine.h"
#include "src/common/channel_id_hash.h"
#include "src/config/config_loader.h"
#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/exchange/rithmic/rithmic_types.h"

// ============================================================================
// Global exit flag — set by SIGTERM handler
// ============================================================================

static volatile sig_atomic_t g_bExit = 0;

static void signal_handler(int) { g_bExit = 1; }

// ============================================================================
// main (matching rithmic_md_saver structure)
// ============================================================================

int main(int argc, char** argv) {
  // Die if parent process exits (prevents orphaned child)
  ::prctl(PR_SET_PDEATHSIG, SIGTERM);

  // ---- Logger init (stdout only, matching rithmic_md_saver bootstrap pattern) ----
  auto* logger = quill::simple_logger("stdout");

  // ---- Config path from positional arg ----
  const char* config_path = (argc > 1) ? argv[1] : "config/config.yaml";

  // ---- Load config ----
  try {
    sqc::Config::Load(config_path);
  } catch(const std::exception& e) {
    LOG_ERROR(logger, "Config error: {}", e.what());
    return 1;
  }

  const auto& cfg = sqc::Config::Instance();
  const auto& rcfg = cfg.rithmic;

  // Read runtime parameters from config.yaml
  const char* shm_name = rcfg.shm_name.c_str();
  uint32_t engine_core = rcfg.engine_core;
  bool cpu_affinity = cfg.global.cpu_affinity;

  LOG_INFO(logger, "rithmic_gateway starting");
  LOG_INFO(logger, "Shm: {}  EngineCore: {}  CpuAffinity: {}", shm_name, engine_core, cpu_affinity);

  // ---- Build channel map directly from config ----
  // channel_id is deterministic: ComputeChannelId(exchange, "futures", symbol)
  using SubEntry = sqc::rithmic::RithmicEngine::SubEntry;

  sqc::rithmic::RithmicChannelMap channel_map;
  std::vector<SubEntry> sub_entries;

  for(const auto& ex : cfg.exchanges) {
    if(!ex.enabled) continue;
    for(const auto& ch : ex.channels) {
      if(ch.type != "futures") continue;
      for(const auto& sym : ch.symbols) {
        if(!sym.enabled) continue;
        uint32_t id = sqc::ComputeChannelId(ex.name, "futures", sym.name);
        channel_map.Register(ex.name, sym.name, id, sym.enabled);
        sub_entries.push_back({sym.enabled, ex.name, sym.name, id});
      }
    }
  }

  if(sub_entries.empty()) {
    LOG_ERROR(logger, "No enabled futures symbols found in config");
    return 1;
  }

  LOG_INFO(logger, "Channel map: {} subscriptions", sub_entries.size());

  // ---- Map shared memory ----
  sqc::rithmic::ShmSetup shm(shm_name);
  auto* hdr = shm.header();
  auto* tick_q = shm.tick_queue();
  auto* depth_q = shm.depth_queue();
  auto* book_ticker_q = shm.book_ticker_queue();

  sqc::rithmic::SsboeConverter converter;

  // ---- RithmicEngine: connection + subscription (matches rithmic_md_saver) ----
  sqc::rithmic::RithmicEngine engine(rcfg, *tick_q, *depth_q, *book_ticker_q, channel_map, converter, sub_entries);

  if(!engine.Start()) {
    return 1;
  }

  // ---- Signal handling: SIGTERM/SIGINT triggers graceful shutdown (unsubscribe+logout) ----
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  // Unblock SIGTERM and SIGINT — the parent process blocks them via
  // pthread_sigmask, and posix_spawn inherits the blocked signal mask.
  // Without this, kill(child_pid, SIGTERM) has no effect until SIGKILL fallback.
  {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
  }

  // Pin engine thread to configured core
  if(cpu_affinity) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(engine_core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
  }

  hdr->child_ready.store(1, std::memory_order_release);

  LOG_INFO(logger, "Receiving market data. Awaiting shutdown signal.");

  // ---- Main loop (matching rithmic_md_saver: sleep(1) + heartbeat) ----
  while(!g_bExit) {
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    hdr->heartbeat_ns.store(static_cast<uint64_t>(now_ns), std::memory_order_release);

    sleep(1);
  }

  LOG_INFO(logger, "Shutting down...");

  // ---- Unsubscribe + logout (matching rithmic_md_saver) ----
  hdr->child_ready.store(0, std::memory_order_release);
  engine.Stop();
  LOG_INFO(logger, "rithmic gateway shutdown complete");
  return 0;
}
