#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// EventType — discriminator for the event union
// ============================================================================

enum class EventType : uint8_t { NONE = 0, TICK = 1, DEPTH = 2, BOOK_TICKER = 3 };

// ============================================================================
// RithmicEvent — fixed-size, trivially copyable event for the MPSC queue.
//
// Contains a type-tagged union of the three output event types. Size is
// dominated by DepthUpdateEvent (~3.2 KB with kMaxOrderbookLevels=100).
// Memory is pre-allocated at system startup per the zero-runtime-allocation
// requirement.
// ============================================================================

struct alignas(64) RithmicEvent {
  EventType type = EventType::NONE;
  uint32_t channel_id = 0;

  union {
    TickData tick;
    BookTickerEvent book_ticker;
    DepthUpdateEvent depth;
  };

  RithmicEvent() noexcept : depth() {}
  RithmicEvent(const RithmicEvent&) = default;
  RithmicEvent& operator=(const RithmicEvent&) = default;
  ~RithmicEvent() = default;
};

// All members are standard-layout types with trivial destructors.
// Safe to memcpy between queue slots despite non-trivial default
// constructors (DepthUpdateEvent has default member initializers).
static_assert(std::is_standard_layout_v<RithmicEvent>,
              "RithmicEvent must be standard layout for lock-free queue");

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

  void Register(std::string_view exchange, std::string_view ticker, uint32_t channel_id) {
    entries_.push_back(Entry{std::string(exchange), std::string(ticker), channel_id});
    sub_list_.push_back(SubEntry{std::string(exchange), std::string(ticker), channel_id});
    sub_count_ = sub_list_.size();
  }

  // ---- Freeze and lookup phase (callbacks run AFTER Freeze) ----

  void Freeze();

  [[nodiscard]] uint32_t Lookup(std::string_view exchange, std::string_view ticker) const noexcept;

  [[nodiscard]] bool IsFrozen() const noexcept { return frozen_; }
  [[nodiscard]] size_t Size() const noexcept { return sub_count_; }

  struct SubEntry { std::string exchange; std::string ticker; uint32_t channel_id; };
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
// Rithmic timestamps are "seconds since beginning of epoch" (SSBOE), which
// means seconds since midnight UTC for the current trading day, plus
// microseconds within that second.
//
// The converter tracks the current reference date (midnight UTC) and detects
// midnight crossings when iSsboe decreases between consecutive calls.
// Thread-safe: multiple REngine callback threads may call concurrently.
// ============================================================================

class SsboeConverter {
 public:
  SsboeConverter();

  /// Convert (seconds_since_midnight, usecs) to microseconds since Unix epoch.
  /// Thread-safe via relaxed atomics. Multiple REngine callback threads may
  /// call concurrently; midnight-crossing detection uses last_ssboe_ as a
  /// heuristic guard — the reference date advances only when the callback
  /// thread that first observes the crossing wins the CAS.
  [[nodiscard]] uint64_t ToEpochMicros(int ssboe, int usecs) noexcept;

 private:
  static constexpr uint64_t kMicrosPerDay = 24ULL * 3600ULL * 1000000ULL;

  // Both fields are updated from callback threads; relaxed atomics suffice
  // because the only invariant is monotonicity of reference_date_micros_.
  std::atomic<uint64_t> reference_date_micros_;
  std::atomic<int> last_ssboe_;
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
  // FNV-1a 64-bit hash, combining both strings with a separator
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

  // Size for ~50% max load factor (power of 2)
  size_t table_size = 1;
  while (table_size < entries_.size() * 2) table_size <<= 1;

  hash_table_.resize(table_size);

  for (const auto& entry : entries_) {
    uint64_t h = Hash(entry.exchange, entry.ticker);
    size_t idx = h & (table_size - 1);

    // Linear probe for empty slot
    while (hash_table_[idx].channel_id != kNotFound) {
      idx = (idx + 1) & (table_size - 1);
    }

    hash_table_[idx] = TableSlot{h, entry.exchange, entry.ticker, entry.channel_id};
  }

  // Free source vector memory — no longer needed after freeze
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

  // Linear probe with a bound (table is never >75% full)
  for (size_t probe = 0; probe < hash_table_.size(); ++probe) {
    const auto& slot = hash_table_[idx];
    if (slot.channel_id == kNotFound) return kNotFound;  // empty slot → miss
    if (slot.hash == h && slot.exchange == exchange && slot.ticker == ticker) {
      return slot.channel_id;
    }
    idx = (idx + 1) & (hash_table_.size() - 1);
  }
  return kNotFound;
}

// ---- SsboeConverter ----

inline SsboeConverter::SsboeConverter() : reference_date_micros_(0), last_ssboe_(-1) {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto now_us = duration_cast<microseconds>(now.time_since_epoch()).count();
  // Truncate to midnight UTC
  uint64_t ref = static_cast<uint64_t>(now_us) -
                 (static_cast<uint64_t>(now_us) % kMicrosPerDay);
  reference_date_micros_.store(ref, std::memory_order_relaxed);
}

inline uint64_t SsboeConverter::ToEpochMicros(int ssboe, int usecs) noexcept {
  // Detect midnight crossing: if ssboe decreased significantly vs last seen,
  // we crossed midnight → advance reference date by 24h.
  int prev = last_ssboe_.load(std::memory_order_relaxed);
  if (prev >= 0 && ssboe < prev - 3600) {
    uint64_t cur = reference_date_micros_.load(std::memory_order_relaxed);
    reference_date_micros_.compare_exchange_strong(cur, cur + kMicrosPerDay,
                                                    std::memory_order_relaxed);
  }
  last_ssboe_.store(ssboe, std::memory_order_relaxed);

  uint64_t ref = reference_date_micros_.load(std::memory_order_relaxed);
  return ref + static_cast<uint64_t>(ssboe) * 1000000ULL +
         static_cast<uint64_t>(usecs);
}

}  // namespace rithmic
}  // namespace sqc
