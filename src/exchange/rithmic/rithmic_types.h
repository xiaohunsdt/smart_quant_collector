#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "src/common/channel_id_hash.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicChannelMap — direct hash lookup for (exchange, ticker) → channel_id.
//
// channel_id is now derived deterministically via ComputeChannelId, so no
// string→id table is needed.  The class only needs to know which (exchange,
// ticker) pairs are registered so that callbacks for unknown tickers can be
// rejected quickly.
// ============================================================================

class RithmicChannelMap {
 public:
  static constexpr uint32_t kNotFound = UINT32_MAX;

  RithmicChannelMap() = default;

  // ---- Population phase (startup only, single-threaded) ----

  void Register(std::string_view exchange, std::string_view ticker, uint32_t channel_id, bool enabled = true) {
    registered_ids_.insert(channel_id);
    sub_list_.push_back(SubEntry{enabled, std::string(exchange), std::string(ticker), channel_id});
  }

  // ---- Lookup — O(1) hash computation + set membership check ----

  [[nodiscard]] uint32_t Lookup(std::string_view exchange, std::string_view ticker) const noexcept {
    uint32_t id = sqc::ComputeChannelId(exchange, "futures", ticker);
    if(registered_ids_.count(id) == 0) return kNotFound;
    return id;
  }

  [[nodiscard]] size_t Size() const noexcept { return sub_list_.size(); }

  struct SubEntry {
    bool enabled = false;
    std::string exchange;
    std::string ticker;
    uint32_t channel_id;
  };
  [[nodiscard]] const std::vector<SubEntry>& Subscriptions() const noexcept { return sub_list_; }

 private:
  std::unordered_set<uint32_t> registered_ids_;
  std::vector<SubEntry> sub_list_;
};

// ============================================================================
// SsboeConverter — converts Rithmic (iSsboe, iUsecs) to epoch microseconds.
//
// Despite the field name "SSBOE" (Seconds Since Beginning Of Epoch), Rithmic
// API's iSsboe is already a full Unix epoch timestamp (seconds since
// 1970-01-01), not seconds since midnight.  The converter just combines the
// two parts so callers don't have to inline the arithmetic.
// ============================================================================

class SsboeConverter {
 public:
  SsboeConverter() = default;

  /// Convert (unix_epoch_seconds, usecs) to microseconds since Unix epoch.
  [[nodiscard]] uint64_t ToEpochMicros(int ssboe, int usecs) noexcept;
};

// ============================================================================
// ResolveRithmicExchangeTimestamp — MD_IMAGE_CB may carry zero timestamps.
//
// Returns false when the snapshot should not be published (empty book, no ts).
// Returns true when out_ts is valid for storage (original or wall-clock fallback).
// ============================================================================

[[nodiscard]] bool ResolveRithmicExchangeTimestamp(uint64_t raw_ts, uint32_t bid_count, uint32_t ask_count, uint64_t& out_ts) noexcept;

}  // namespace rithmic
}  // namespace sqc

// ============================================================================
// Inline implementations
// ============================================================================

namespace sqc {
namespace rithmic {

inline uint64_t SsboeConverter::ToEpochMicros(int ssboe, int usecs) noexcept {
  return static_cast<uint64_t>(ssboe) * 1000000ULL + static_cast<uint64_t>(usecs);
}

inline bool ResolveRithmicExchangeTimestamp(uint64_t raw_ts, uint32_t bid_count, uint32_t ask_count, uint64_t& out_ts) noexcept {
  if(raw_ts != 0) {
    out_ts = raw_ts;
    return true;
  }
  if(bid_count == 0 && ask_count == 0) {
    return false;
  }
  out_ts = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
  return true;
}

}  // namespace rithmic
}  // namespace sqc
