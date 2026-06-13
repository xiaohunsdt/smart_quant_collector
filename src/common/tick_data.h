#pragma once

#include <cstdint>

namespace sqc {

// 64-byte cache-line aligned, per spec §2.1
struct alignas(64) TickData {
  uint64_t exchange_timestamp;  // exchange timestamp in microseconds since epoch
  uint64_t local_diff;          // end-to-end latency (receive→disk) in ns (CLOCK_MONOTONIC)
  uint64_t trade_id;            // trade ID
  double price;
  double quantity;
  uint32_t channel_id;  // channel/symbol mapping ID
  char symbol[12];      // symbol name (max 11 chars + null). sizeof(TickData)==64 requires
                        // symbol <= 12 bytes (1 cache line on hot path).
                        // Symbols longer than 11 chars are rejected at config load time
                        // (see config_loader.cpp ParseSymbol).
  bool is_buyer_maker;  // true = Sell (maker is buyer), false = Buy
  char padding[3];      // explicit padding for natural alignment
};

static_assert(sizeof(TickData) == 64, "TickData size must be exactly 64 bytes");

}  // namespace sqc
