#pragma once

#include <charconv>
#include <string_view>

namespace sqc {

// Converts a string_view to double.
// Uses std::from_chars — no null-termination needed, no locale overhead.
inline double SvToDouble(std::string_view sv) {
  double result = 0.0;
  std::from_chars(sv.data(), sv.data() + sv.size(), result);
  return result;
}

}  // namespace sqc
