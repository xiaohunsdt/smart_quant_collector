#pragma once

#include <cstdint>
#include <cstring>

#include "src/orderbook/orderbook_event.h"

namespace sqc {

/// Fixed-width binary record layouts for mmap persistence. Single source of
/// truth for the orderbook / bookticker on-disk format — previously these two
/// structs were defined inline in storage_router.cpp and populated by hand in
/// four separate places (RouteOrderbook, RouteBookTicker, WriteOrderbookToMmap,
/// WriteBookTickerToMmap), byte-for-byte identically.
namespace record_layout {

/// Fixed-width symbol field used by every binary record. Centralized so the
/// truncation/bounds check lives in one place instead of bare memcpy() calls.
constexpr size_t kSymbolFieldBytes = 32;

/// Copy at most kSymbolFieldBytes-1 bytes from a (possibly non-NUL-terminated)
/// fixed-size source into a fixed-width destination and NUL-terminate it.
/// `src_size` is the capacity of the source buffer (e.g. sizeof(TickData::symbol)),
/// not its string length.
inline void CopySymbol(char (&dst)[kSymbolFieldBytes], const char* src, size_t src_size) noexcept {
  const size_t n = (src_size < kSymbolFieldBytes) ? src_size : kSymbolFieldBytes;
  std::memcpy(dst, src, n);
  if (n < kSymbolFieldBytes) dst[n] = '\0';
  else dst[kSymbolFieldBytes - 1] = '\0';
}

/// Binary header preceding a variable-length orderbook record in the mmap
/// stream, followed by `bid_count + ask_count` PriceLevel entries.
struct alignas(8) OrderbookRecordHeader {
  uint64_t exchange_timestamp;
  uint64_t local_diff;
  char symbol[kSymbolFieldBytes];
  uint32_t bid_count;
  uint32_t ask_count;
};

/// Fixed-width bookticker record.
struct alignas(8) BookTickerRecord {
  uint64_t exchange_timestamp;
  uint64_t local_diff;
  char symbol[kSymbolFieldBytes];
  double best_bid_price;
  double best_bid_qty;
  double best_ask_price;
  double best_ask_qty;
};

}  // namespace record_layout

}  // namespace sqc
