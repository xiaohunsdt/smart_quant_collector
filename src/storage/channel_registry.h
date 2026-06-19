#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

/// Per-channel metadata, frozen at startup. Resolved on the hot path without
/// copying (callers hold const references into this struct).
/// Hoisted out of DolphinDBClient so the registry is shared cleanly across
/// backends and is testable in isolation.
struct ChannelMeta {
  std::string exchange;
  std::string market_type;
  std::string symbol;
  uint32_t depth_level = 0;  // 0 = unset; populated from config.yaml per symbol
};

/// Append-then-freeze registry mapping channel_id -> ChannelMeta.
///
/// Lifecycle contract:
///   1. During startup (single-threaded), producers call Register() for each
///      channel.
///   2. main() calls Freeze() before parser threads start.
///   3. After Freeze(), hot-path threads call Find() concurrently without
///      locking (the map is immutable).
///
/// Concurrency fix vs. the old DolphinDBClient: the previous channels_frozen_
/// flag only asserted in Debug builds, so a late Register() in Release could
/// silently mutate the map under concurrent readers. Now Register() refuses
/// writes (with an error log) once frozen in ALL builds.
class StorageChannelRegistry {
 public:
  /// Register metadata for a channel. Refuses to write once frozen (logs an
  /// error and returns) — see the class doc for why this is enforced in
  /// Release too.
  void Register(uint32_t channel_id, std::string exchange, std::string market_type, std::string symbol, uint32_t depth_level);

  /// Seal the registry. Subsequent Register() calls are rejected. Must be
  /// called single-threaded before any Find() from hot-path threads.
  void Freeze() noexcept { frozen_.store(true, std::memory_order_release); }

  [[nodiscard]] bool IsFrozen() const noexcept { return frozen_.load(std::memory_order_acquire); }

  /// Look up channel metadata. Safe to call concurrently after Freeze().
  /// Returns nullptr for an unknown channel_id.
  [[nodiscard]] const ChannelMeta* Find(uint32_t channel_id) const noexcept {
    const auto it = channels_.find(channel_id);
    return it != channels_.end() ? &it->second : nullptr;
  }

  [[nodiscard]] size_t Size() const noexcept { return channels_.size(); }

 private:
  std::unordered_map<uint32_t, ChannelMeta> channels_;
  std::atomic<bool> frozen_{false};
};

}  // namespace sqc
