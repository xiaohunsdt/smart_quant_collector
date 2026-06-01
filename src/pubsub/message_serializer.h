#pragma once

#include <cstdint>
#include <cstring>

#include "pubsub/pub_message.h"
#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

// ── Compile-time serialization strategy ────────────────────────────
//
// Strategy pattern without virtual functions. The template parameters
// (message type tag + C++ struct type) are fixed at compile time, so
// all calls are fully inlined with zero overhead.
//
// Wire format:  [1-byte PubMsgType tag] [sizeof(T) bytes of struct]
//
// Adding a new message type requires only a new type alias — no
// existing code needs to change (Open/Closed Principle).
template <PubMsgType kType, typename T>
struct MessageSerializer {
  static constexpr PubMsgType type = kType;
  static constexpr size_t wire_size = sizeof(T) + 1;
  using value_type = T;

  // Serialize src into dst. Caller must ensure dst has at least
  // wire_size bytes available.
  static void serialize(void* dst, const T& src) noexcept {
    auto* ptr = static_cast<uint8_t*>(dst);
    ptr[0] = static_cast<uint8_t>(kType);
    std::memcpy(ptr + 1, &src, sizeof(T));
  }
};

// ── Type aliases — one per message type ───────────────────────────
using TickSerializer =
    MessageSerializer<PubMsgType::kTick, TickData>;
using DepthSerializer =
    MessageSerializer<PubMsgType::kDepth, DepthUpdateEvent>;
using BookTickerSerializer =
    MessageSerializer<PubMsgType::kBookTicker, BookTickerEvent>;

}  // namespace sqc
