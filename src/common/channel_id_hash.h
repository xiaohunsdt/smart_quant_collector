#pragma once

#include <cstdint>
#include <string_view>

namespace sqc {

/// Deterministic channel ID: FNV-1a over "exchange:type_name:symbol",
/// folded to 32 bits.  Both the main process and rithmic_gateway use this
/// header so IDs are identical across process boundaries without any IPC.
///
/// type_name must be the canonical string: "spot", "perpetual", or "futures".
[[nodiscard]] constexpr uint32_t ComputeChannelId(std::string_view exchange, std::string_view type_name, std::string_view symbol) noexcept {
  constexpr uint64_t kBasis = 14695981039346656037ULL;
  constexpr uint64_t kPrime = 1099511628211ULL;

  uint64_t h = kBasis;
  for(unsigned char c : exchange) {
    h ^= c;
    h *= kPrime;
  }
  h ^= static_cast<uint64_t>(':');
  h *= kPrime;
  for(unsigned char c : type_name) {
    h ^= c;
    h *= kPrime;
  }
  h ^= static_cast<uint64_t>(':');
  h *= kPrime;
  for(unsigned char c : symbol) {
    h ^= c;
    h *= kPrime;
  }

  // Fold 64-bit hash to 32 bits.
  return static_cast<uint32_t>(h ^ (h >> 32));
}

}  // namespace sqc
