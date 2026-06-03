#include "cpu_affinity.h"

#include <thread>

#ifdef __linux__
#include <pthread.h>
#elif defined(__APPLE__)
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#endif

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

bool PinToCore(uint32_t core_id) {
#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if(rc != 0) {
    LOG_WARNING(GetLogger(), "Failed to pin thread to core {}: {}", core_id, rc);
    return false;
  } else {
    LOG_INFO(GetLogger(), "Thread pinned to core {}", core_id);
    return true;
  }
#elif defined(__APPLE__)
  LOG_WARNING(GetLogger(), "CPU pinning not supported on macOS (requested core {})", core_id);
  return false;
#else
  LOG_WARNING(GetLogger(), "CPU pinning not supported on this platform");
  return false;
#endif
}

std::vector<uint32_t> GetAvailableCores() {
  unsigned int n = std::thread::hardware_concurrency();
  std::vector<uint32_t> cores;
  cores.reserve(n);
  for(uint32_t i = 0; i < n; ++i) {
    cores.push_back(i);
  }
  return cores;
}

}  // namespace sqc
