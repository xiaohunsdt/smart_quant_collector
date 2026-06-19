#pragma once

#include <cstdint>
#include <utility>

#include "simdjson.h"
#include "src/common/string_utils.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

// ---------------------------------------------------------------------------
// Shared crypto parsing helpers
//
// These are header-only, inline, noexcept-where-possible utilities extracted
// from the per-exchange parsers to eliminate repeated JSON-walking idioms.
// They never allocate and are safe to call on the parser-thread hot path.
// ---------------------------------------------------------------------------

// Convert a millisecond timestamp to microseconds.  Several exchanges report
// ms (e.g. Binance futures, Gate.io) while the rest of the pipeline works in
// μs; centralising the *1000 makes the unit conversion explicit at every
// call site rather than a bare magic multiplier.
constexpr uint64_t MsToUs(uint64_t ms) noexcept { return ms * 1000ULL; }

// Named delays (ms) used when staggering subscription messages on a freshly
// opened WebSocket.  Centralised so every exchange/module uses the same value
// and the intent is documented.
constexpr uint32_t kDepthSubscribeDelayMs = 500;
constexpr uint32_t kBookTickerSubscribeDelayMs = 700;

// Walk a positional level array of the form [["price","qty",...], ...] and fill
// up to `max_levels` PriceLevels into `out`.  This is the shape used by Binance
// (spot partial depth + futures depthUpdate) and Gate.io spot order_book.
//
// Each inner level's first two elements are read positionally via an iterator,
// tolerating the trailing ignore-array Binance emits on spot partial depth.
//
// ERROR SEMANTICS: a level whose price or quantity fails to parse (non-numeric,
// truncated, or missing element) is SKIPPED entirely — it is not counted in the
// return value and is not written to `out`. As a result the successfully parsed
// levels are compacted to the front of `out` and the returned count may be less
// than `max_levels`. This is a deliberate improvement over the pre-refactor
// Binance perpetual behavior, which wrote a {0.0, 0.0} placeholder and counted
// it (publishing spurious zero levels into the book). Downstream consumers must
// honor the returned count rather than assuming `max_levels` non-null entries:
//   - CsvWriter pads missing tail slots with 0.0 up to the header width.
//   - MmapBackend writes a variable-length record keyed on bid_count/ask_count.
//   - DolphinDBClient fills unfilled slots with 0.0 up to depth_level.
//
// Returns the number of levels successfully parsed. NOT noexcept: simdjson
// traversal may throw simdjson_error, which the caller's surrounding try/catch
// (matching the pre-refactor behaviour) is expected to handle.
inline uint32_t ParsePositionalLevelArray(simdjson::ondemand::array arr, PriceLevel* out, uint32_t max_levels) {
  uint32_t count = 0;
  for(auto level : arr) {
    if(count >= max_levels) break;
    auto it = level.begin();
    if(it == level.end()) continue;
    double p = 0.0, q = 0.0;
    if(!SvToDouble((*it).get_string(), p)) continue;
    ++it;
    if(it == level.end()) continue;
    if(!SvToDouble((*it).get_string(), q)) continue;
    out[count++] = {p, q};
  }
  return count;
}

// Helper for the repeated dispatch idiom in every per-exchange Parse():
//   result.type = <type>;
//   if(!parse_fn(...)) result.type = ParsedType::NONE;
// `parse_fn` must return bool (true = success).  Kept as a template so it
// inlines completely and never appears on a non-hot path.
template <typename Fn>
inline void DispatchParse(ParseResult& result, ParsedType type, Fn&& parse_fn) {
  result.type = type;
  if(!std::forward<Fn>(parse_fn)()) result.type = ParsedType::NONE;
}

}  // namespace sqc
