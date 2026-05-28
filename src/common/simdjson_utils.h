#pragma once

#include <cstddef>

#include "simdjson.h"

namespace sqc {

// simdjson requires SIMDJSON_PADDING bytes of readable buffer after JSON data
constexpr size_t kSimdjsonPadding = simdjson::SIMDJSON_PADDING;

}  // namespace sqc
