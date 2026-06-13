#include "quill/LogMacros.h"
#include "quill/SimpleSetup.h"

#include <csignal>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#ifdef WinOS
#include <windows.h>
#define sleep(s) Sleep((s) * 1000)
#define signal(s, h) signal(s, h)
#else
#include <unistd.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/prctl.h>

#include "src/config/config_loader.h"
#include "rithmic_engine.h"
#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/exchange/rithmic/rithmic_types.h"

// ============================================================================
// Global exit flag — set by SIGTERM handler
// ============================================================================

static volatile sig_atomic_t g_bExit = 0;

static void signal_handler(int) {
    g_bExit = 1;
}

// ============================================================================
// Channel map deserialization (read from stdin pipe at startup)
// ============================================================================

namespace {

using SubEntry = sqc::rithmic::RithmicEngine::SubEntry;

std::vector<SubEntry> ReadChannelMapFromStdin() {
    std::vector<SubEntry> result;
    uint32_t count = 0;

    if (::read(STDIN_FILENO, &count, sizeof(count)) != static_cast<ssize_t>(sizeof(count))) {
        return result;
    }

    for (uint32_t i = 0; i < count; ++i) {
        SubEntry entry;

        uint8_t exchange_len = 0;
        if (::read(STDIN_FILENO, &exchange_len, sizeof(exchange_len)) != sizeof(exchange_len)) break;

        std::string exchange(exchange_len, '\0');
        if (::read(STDIN_FILENO, exchange.data(), exchange_len) != static_cast<ssize_t>(exchange_len)) break;
        entry.exchange = exchange;

        uint8_t ticker_len = 0;
        if (::read(STDIN_FILENO, &ticker_len, sizeof(ticker_len)) != sizeof(ticker_len)) break;

        std::string ticker(ticker_len, '\0');
        if (::read(STDIN_FILENO, ticker.data(), ticker_len) != static_cast<ssize_t>(ticker_len)) break;
        entry.ticker = ticker;

        if (::read(STDIN_FILENO, &entry.channel_id, sizeof(entry.channel_id)) != sizeof(entry.channel_id)) break;

        uint8_t enabled_byte = 0;
        if (::read(STDIN_FILENO, &enabled_byte, sizeof(enabled_byte)) != sizeof(enabled_byte)) break;
        entry.enabled = (enabled_byte != 0);

        result.push_back(std::move(entry));
    }

    ::close(STDIN_FILENO);
    return result;
}

}  // namespace

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
    } catch (const std::exception& e) {
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
    LOG_INFO(logger, "Shm: {}  EngineCore: {}  CpuAffinity: {}",
             shm_name, engine_core, cpu_affinity);

    // ---- Map shared memory + read channel map from stdin ----
    sqc::rithmic::ShmSetup shm(shm_name);
    auto* hdr = shm.header();
    auto* tick_q = shm.tick_queue();
    auto* depth_q = shm.depth_queue();
    auto* book_ticker_q = shm.book_ticker_queue();

    auto sub_entries = ReadChannelMapFromStdin();
    if (sub_entries.empty()) {
        LOG_ERROR(logger, "Failed to read channel map from stdin");
        return 1;
    }

    sqc::rithmic::RithmicChannelMap channel_map;
    for (const auto& sub : sub_entries) {
        channel_map.Register(sub.exchange, sub.ticker, sub.channel_id);
    }
    channel_map.Freeze();

    sqc::rithmic::SsboeConverter converter;

    LOG_INFO(logger, "Channel map: {} subscriptions", sub_entries.size());

    // ---- RithmicEngine: connection + subscription (matches rithmic_md_saver) ----
    sqc::rithmic::RithmicEngine engine(rcfg, *tick_q, *depth_q, *book_ticker_q,
                                        channel_map, converter, sub_entries);

    if (!engine.Start()) {
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
    if (cpu_affinity) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(engine_core, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
    }

    hdr->child_ready.store(1, std::memory_order_release);

    LOG_INFO(logger, "Receiving market data. Awaiting shutdown signal.");

    // ---- Main loop (matching rithmic_md_saver: sleep(1) + heartbeat) ----
    while (!g_bExit) {
        auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        hdr->heartbeat_ns.store(static_cast<uint64_t>(now_ns), std::memory_order_release);

        sleep(1);
    }

    LOG_INFO(logger, "Shutting down...");

    // ---- Unsubscribe + logout (matching rithmic_md_saver) ----
    hdr->child_ready.store(0, std::memory_order_release);
    engine.Stop();
    LOG_INFO(logger, "Done.");
    return 0;
}
