#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicChannelMap — frozen hash map for (exchange, ticker) → channel_id.
//
// Populated at startup (before REngine callbacks begin), then frozen.
// Hot-path Lookup() is lock-free: no mutex, no atomic RMW, just loads.
//
// Uses linear probing in a flat array for cache-friendly O(1) lookup.
// ============================================================================

class RithmicChannelMap {
 public:
  static constexpr uint32_t kNotFound = UINT32_MAX;

  RithmicChannelMap() = default;

  // ---- Population phase (startup only, single-threaded) ----

  void Register(std::string_view exchange, std::string_view ticker, uint32_t channel_id, bool enabled = true) {
    entries_.push_back(Entry{std::string(exchange), std::string(ticker), channel_id});
    sub_list_.push_back(SubEntry{enabled, std::string(exchange), std::string(ticker), channel_id});
    sub_count_ = sub_list_.size();
  }

  // ---- Freeze and lookup phase (callbacks run AFTER Freeze) ----

  void Freeze();

  [[nodiscard]] uint32_t Lookup(std::string_view exchange, std::string_view ticker) const noexcept;

  [[nodiscard]] bool IsFrozen() const noexcept { return frozen_; }
  [[nodiscard]] size_t Size() const noexcept { return sub_count_; }

  struct SubEntry { bool enabled = false; std::string exchange; std::string ticker; uint32_t channel_id; };
  [[nodiscard]] const std::vector<SubEntry>& Subscriptions() const noexcept { return sub_list_; }

 private:
  struct TableSlot {
    uint64_t hash = 0;
    std::string exchange;
    std::string ticker;
    uint32_t channel_id = kNotFound;
  };

  static uint64_t Hash(std::string_view exchange, std::string_view ticker) noexcept;

  void BuildHashTable();

  struct Entry {
    std::string exchange;
    std::string ticker;
    uint32_t channel_id;
  };

  std::vector<Entry> entries_;
  std::vector<TableSlot> hash_table_;
  std::vector<SubEntry> sub_list_;
  size_t sub_count_ = 0;
  bool frozen_ = false;
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

}  // namespace rithmic
}  // namespace sqc

// ============================================================================
// Inline implementations
// ============================================================================

namespace sqc {
namespace rithmic {

// ---- RithmicChannelMap ----

inline uint64_t RithmicChannelMap::Hash(std::string_view exchange,
                                         std::string_view ticker) noexcept {
  uint64_t h = 14695981039346656037ULL;
  for (char c : exchange) {
    h ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
    h *= 1099511628211ULL;
  }
  h ^= static_cast<uint64_t>(':');
  h *= 1099511628211ULL;
  for (char c : ticker) {
    h ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
    h *= 1099511628211ULL;
  }
  return h;
}

inline void RithmicChannelMap::BuildHashTable() {
  if (entries_.empty()) return;

  size_t table_size = 1;
  while (table_size < entries_.size() * 2) table_size <<= 1;

  hash_table_.resize(table_size);

  for (const auto& entry : entries_) {
    uint64_t h = Hash(entry.exchange, entry.ticker);
    size_t idx = h & (table_size - 1);

    while (hash_table_[idx].channel_id != kNotFound) {
      idx = (idx + 1) & (table_size - 1);
    }

    hash_table_[idx] = TableSlot{h, entry.exchange, entry.ticker, entry.channel_id};
  }

  entries_.clear();
  entries_.shrink_to_fit();
}

inline void RithmicChannelMap::Freeze() {
  if (frozen_) return;
  BuildHashTable();
  frozen_ = true;
}

inline uint32_t RithmicChannelMap::Lookup(std::string_view exchange,
                                           std::string_view ticker) const noexcept {
  if (!frozen_ || hash_table_.empty()) return kNotFound;

  uint64_t h = Hash(exchange, ticker);
  size_t idx = h & (hash_table_.size() - 1);

  for (size_t probe = 0; probe < hash_table_.size(); ++probe) {
    const auto& slot = hash_table_[idx];
    if (slot.channel_id == kNotFound) return kNotFound;
    if (slot.hash == h && slot.exchange == exchange && slot.ticker == ticker) {
      return slot.channel_id;
    }
    idx = (idx + 1) & (hash_table_.size() - 1);
  }
  return kNotFound;
}

// ---- SsboeConverter ----

inline uint64_t SsboeConverter::ToEpochMicros(int ssboe, int usecs) noexcept {
  return static_cast<uint64_t>(ssboe) * 1000000ULL +
         static_cast<uint64_t>(usecs);
}

}  // namespace rithmic
}  // namespace sqc
