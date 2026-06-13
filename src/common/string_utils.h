#pragma once

#include <charconv>
#include <cstring>
#include <string_view>

namespace sqc {

// Parse a decimal string into `out` using std::from_chars (no allocation, no locale).
// Returns true on success. On failure `out` is left unchanged.
[[nodiscard]] inline bool SvToDouble(std::string_view sv, double& out) noexcept {
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
  return ec == std::errc{};
}

// Copy `src` into the fixed null-terminated buffer `dst[0..dst_size-1]`.
// Returns true if `src` was truncated to fit; the caller should log a warning.
[[nodiscard]] inline bool CopySymbol(char* dst, size_t dst_size, std::string_view src) noexcept {
  const bool truncated = src.size() >= dst_size;
  const size_t len = truncated ? dst_size - 1 : src.size();
  std::memcpy(dst, src.data(), len);
  dst[len] = '\0';
  return truncated;
}

}  // namespace sqc
