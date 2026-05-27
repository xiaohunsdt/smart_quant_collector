#include "signal_handler.h"

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>

namespace sqc {

namespace {
std::atomic<bool> shutdown_requested{false};
std::mutex cv_mutex;
std::condition_variable cv;
bool notified = false;
}  // namespace

void SignalHandler::Install() {
  shutdown_requested = false;
  notified = false;

  auto handler = [](int /*signum*/) {
    shutdown_requested = true;
    {
      std::lock_guard<std::mutex> lock(cv_mutex);
      notified = true;
    }
    cv.notify_all();
  };

  std::signal(SIGINT, handler);
  std::signal(SIGTERM, handler);
}

bool SignalHandler::IsShutdownRequested() { return shutdown_requested; }

void SignalHandler::WaitForShutdown() {
  std::unique_lock<std::mutex> lock(cv_mutex);
  cv.wait(lock, [] { return notified; });
}

void SignalHandler::Reset() {
  shutdown_requested = false;
  {
    std::lock_guard<std::mutex> lock(cv_mutex);
    notified = false;
  }
}

}  // namespace sqc
