#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/channel_registry.h"

namespace sqc {

/// Strategy interface implemented by every storage backend (csv / mmap /
/// dolphindb). Eliminates the 9-way `if(use_engine_=="...")` string dispatch
/// that used to live in StorageRouter — the router now holds a single
/// unique_ptr<IStorageBackend> and delegates polymorphically.
///
/// Threading contract (matches the router's public API):
///   - RegisterChannel() / FreezeChannels() are called single-threaded during
///     startup, before any Route*.
///   - InsertTick/InsertOrderbook/InsertBookTicker are called concurrently
///     from many parser threads after FreezeChannels(). Implementations must
///     be internally synchronized.
///   - FlushAndClose() is called once after all producers have stopped.
///
/// Hot-path cost: one virtual dispatch per Insert* call (~1ns indirect
/// branch). This is negligible versus the existing per-call string compares,
/// and utterly negligible on the dolphindb path (network I/O, ms) and the
/// csv/mmap paths (already mutex-serialized).
class IStorageBackend {
 public:
  virtual ~IStorageBackend() = default;

  /// Register a channel for this backend. Strings are already resolved
  /// (exchange name, market type) by the caller (StorageRouter), so backends
  /// stay free of ChannelTypeName/exchange_adapter dependencies.
  virtual void RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type,
                               std::string_view symbol, uint32_t depth_level) = 0;

  /// Seal channel registration. Default no-op for backends that don't care.
  virtual void FreezeChannels() {}

  /// Insert a trade tick.
  virtual void InsertTick(const TickData& tick) = 0;

  /// Insert an orderbook depth update. `local_ts` is the latency in ns.
  /// `depth_level` is the channel's configured depth (for csv header / mmap).
  virtual void InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level) = 0;

  /// Insert a book-ticker snapshot.
  virtual void InsertBookTicker(const BookTickerEvent& event) = 0;

  /// Flush any buffered data and release resources. Called once at shutdown.
  virtual void FlushAndClose() = 0;

  /// Short backend name for logging (e.g. "csv", "mmap", "dolphindb").
  virtual const char* Name() const noexcept = 0;
};

}  // namespace sqc
