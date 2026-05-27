#pragma once

#include <cstddef>

#include "simdjson.h"

namespace sqc {

// simdjson requires SIMDJSON_PADDING bytes of readable buffer after JSON data
constexpr size_t kSimdjsonPadding = simdjson::SIMDJSON_PADDING;

// Maximum JSON message size for exchange WebSocket frames
constexpr size_t kMaxJsonMessageSize = 1024 * 1024;  // 1 MB

}  // namespace sqc
