#include "signal_handler.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <pthread.h>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

namespace {
std::atomic<bool> shutdown_requested{false};
}  // namespace

void SignalHandler::Install() {
  shutdown_requested = false;

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) != 0) {
    if (auto* log = GetLogger())
      LOG_ERROR(log, "SignalHandler: pthread_sigmask failed");
  }
}

bool SignalHandler::IsShutdownRequested() { return shutdown_requested; }

void SignalHandler::WaitForShutdown() {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  int sig = 0;
  while (sigwait(&set, &sig) != 0) {
    if (errno != EINTR) {
      if (auto* log = GetLogger())
        LOG_ERROR(log, "SignalHandler: sigwait failed (errno={})", errno);
      return;
    }
  }
  shutdown_requested = true;
}

void SignalHandler::Reset() { shutdown_requested = false; }

}  // namespace sqc
