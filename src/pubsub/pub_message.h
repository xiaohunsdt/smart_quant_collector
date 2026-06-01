#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "src/orderbook/orderbook_event.h"

namespace sqc {

// ── Wire-format type tag ───────────────────────────────────────────
// Prefixed to every ZMQ payload frame so downstream consumers can
// distinguish TickData, DepthUpdateEvent, and BookTickerEvent
// without probing the payload.
enum class PubMsgType : uint8_t {
  kTick = 1,
  kDepth = 2,
  kBookTicker = 3,
};

// Maximum ZMQ payload frame size in bytes.
// DepthUpdateEvent is the largest: ~3252 bytes + 1-byte type tag.
constexpr size_t kMaxPayloadSize = sizeof(DepthUpdateEvent) + 1;

// Maximum topic string length (format: "exchange:type:symbol:event").
// Longest real topic: "binance:perpetual:BTCUSDT:book_ticker" = 38 chars.
// 64 bytes provides generous headroom.
constexpr size_t kMaxTopicLen = 64;

// ── BuildTopic ─────────────────────────────────────────────────────
// Pure function: formats a ZMQ subscription topic string into a
// caller-provided buffer. Zero heap allocation.
//
// Topic format: "{exchange}:{channel_type}:{symbol}:{event_type}"
// Example:      "binance:spot:BTCUSDT:tick"
//
// Returns the number of bytes written (excluding null terminator).
// Returns -1 if the buffer is too small (truncation would occur).
int64_t BuildTopic(char* buf, size_t buf_size,
                   std::string_view exchange,
                   std::string_view channel_type,
                   std::string_view symbol,
                   std::string_view event_type);

}  // namespace sqc
