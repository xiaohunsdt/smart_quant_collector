#include "cpu_affinity.h"

#include <thread>

#ifdef __linux__
#include <pthread.h>
#elif defined(__APPLE__)
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#endif

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

void PinToCore(uint32_t core_id) {
#ifdef __linux__
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
    LOG_WARNING(GetLogger(), "Failed to pin thread to core {}: {}", core_id, rc);
  } else {
    LOG_INFO(GetLogger(), "Thread pinned to core {}", core_id);
  }
#elif defined(__APPLE__)
  // macOS does not support pthread_setaffinity_np.
  // thread_policy_set provides QoS hints but not hard pinning.
  LOG_WARNING(GetLogger(),
              "CPU pinning not supported on macOS (requested core {})", core_id);
#else
  LOG_WARNING(GetLogger(), "CPU pinning not supported on this platform");
#endif
}

std::vector<uint32_t> GetAvailableCores() {
  unsigned int n = std::thread::hardware_concurrency();
  std::vector<uint32_t> cores;
  cores.reserve(n);
  for (uint32_t i = 0; i < n; ++i) {
    cores.push_back(i);
  }
  return cores;
}

}  // namespace sqc
