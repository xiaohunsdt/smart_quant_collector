#pragma once

#include <cstdint>

#include "tick_data.h"

namespace sqc {

// 72 bytes compact on Linux x64, may be larger on ARM64 due to ABI alignment.
// Use sizeof(StorageTickEnvelope) instead of hardcoded constant in all code paths.
struct StorageTickEnvelope {
  TickData data;             // 64 bytes cache-line aligned (bytes 0-63)
  uint32_t storage_target;   // 0 = default full persist, 1 = DolphinDB degraded data
  uint32_t recovery_status;  // 0 = not recovered, 1 = recovered successfully
};

// On platforms where alignof(TickData)==64 propagates to enclosing struct size,
// the size will be 128. The mmap engine uses sizeof() at runtime for correctness.
static_assert(sizeof(StorageTickEnvelope) == 72 || sizeof(StorageTickEnvelope) == 128, "StorageTickEnvelope: unexpected size");

}  // namespace sqc
