#pragma once

#include <DolphinDB.h>
#include <MultithreadedTableWriter.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "src/common/tick_data.h"
#include "src/config/secure_string.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/channel_registry.h"
#include "storage/client/dolphindb_schema.h"

namespace sqc {

/// DolphinDB storage client using MultithreadedTableWriter (MTW).
///
/// Architecture: C++ API -> Memory Stream Table -> subscribeTable -> DFS TSDB
/// Three MTW instances (trades / orderbook / bookticker) with configurable:
///   batchSize, throttle, threadCount (from config.yaml)
///
/// Schema is auto-created at connect time (InitSchema) if
/// config.storage.dolphindb.auto_init_schema is true.
///
/// Orderbook uses Array Vector (DOUBLE[]) for bid/ask prices & sizes.
class DolphinDBClient {
 public:
  DolphinDBClient();
  ~DolphinDBClient();

  DolphinDBClient(const DolphinDBClient&) = delete;
  DolphinDBClient& operator=(const DolphinDBClient&) = delete;
  DolphinDBClient(DolphinDBClient&&) = delete;
  DolphinDBClient& operator=(DolphinDBClient&&) = delete;

  /// Connect to DolphinDB, auto-initialize schema (if enabled), and
  /// initialize three MTW writers. Config is read from Config::Instance().
  bool Connect(const std::string& host, uint16_t port, const std::string& user, const std::string& password);

  /// Disconnect and flush all pending data via waitForThreadCompletion().
  void Disconnect();

  /// Check if the connection is alive. Performs a lightweight ping
  /// (1+1) throttled by health_check_interval_ms from config.
  [[nodiscard]] bool IsHealthy();

  /// Attempt to reconnect (tears down and recreates MTW instances).
  bool Reconnect();

  /// Register channel metadata so that channel_id can be mapped to
  /// exchange / market_type / symbol / depth_level at insert time.
  void RegisterChannel(uint32_t channel_id, std::string exchange, std::string market_type, std::string symbol, uint32_t depth_level = 0);

  /// Mark channel registration as complete. Delegates to the embedded
  /// StorageChannelRegistry, which now rejects late registrations in ALL builds
  /// (was Debug-only — a real race in Release).
  void FreezeChannels() { registry_.Freeze(); }

  /// Batch insert trades into stream table 'trades_stream'.
  [[nodiscard]] bool TableInsertTrades(const std::vector<TickData>& batch);

  /// Insert orderbook snapshot into stream table 'orderbook_stream'.
  /// Uses Array Vector (DOUBLE[]) for bid/ask prices & sizes.
  /// depth_level is resolved from channel metadata (RegisterChannel).
  [[nodiscard]] bool TableInsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts);

  /// Insert bookticker into stream table 'bookticker_stream'.
  [[nodiscard]] bool TableInsertBookTicker(const BookTickerEvent& event);

  /// Validate that stream tables, DFS tables, and subscriptions are healthy.
  /// Returns true if all tables are present and subscriptions are active.
  [[nodiscard]] bool ValidateSchema();

  // Note: timestamp validation helpers (UsecToDateInt / IsValidTradeDate /
  // IsValidExchangeTimestamp) live in storage/timestamp_util.h, not here —
  // call timestamp_util::... directly. Keeping them off this DB-client header
  // avoids coupling storage-independent validation to a database client class.

 private:
  /// Create database, DFS tables, stream tables, and subscriptions
  /// if they do not already exist (idempotent). Delegates the DDL work to the
  /// DolphinDBSchema component (extracted from the old 167-line inline impl).
  bool InitSchema();

  bool InitWriters();
  void DestroyWriters(bool skip_drain = false);

  /// Resolve channel metadata for an insert. Returns the registered ChannelMeta
  /// (exchange/market_type/symbol/depth_level are stable after FreezeChannels,
  /// so callers may hold const references into it without copying). Returns
  /// nullptr when the channel_id was never registered.
  const ChannelMeta* ResolveChannel(uint32_t channel_id) const noexcept { return registry_.Find(channel_id); }

  std::string host_;
  uint16_t port_ = 0;
  std::string user_;
  SecureString password_;
  std::atomic<bool> connected_{false};
  // last_health_check_ as raw ticks: the old std::chrono::time_point was a
  // non-trivially-sized value read/written from multiple threads under only a
  // *shared* lock (hot-path TableInsert* update it) — a genuine data race.
  // Atomic ticks close the race without changing IsHealthy()'s throttling.
  std::atomic<std::chrono::steady_clock::rep> last_health_check_ns_{0};

  // conn_ is NOT guarded by an internal mutex. All callers (StorageRouter)
  // serialize access via dolphindb_mtx_ (shared_lock for hot-path
  // inserts, unique_lock for Reconnect/Disconnect), and the startup path
  // (Connect) runs single-threaded before parser threads start.
  std::unique_ptr<dolphindb::DBConnection> conn_;

  // Schema DDL component (database/table/stream/subscription init + partition
  // range extension). Stateless; extracted from the old inline InitSchema.
  DolphinDBSchema schema_;

  // Channel registry: populated via RegisterChannel() during startup, then
  // frozen. Insert methods read it concurrently from parser threads.
  StorageChannelRegistry registry_;

  std::unique_ptr<dolphindb::MultithreadedTableWriter> trades_writer_;
  std::unique_ptr<dolphindb::MultithreadedTableWriter> ob_writer_;
  std::unique_ptr<dolphindb::MultithreadedTableWriter> bt_writer_;
};

}  // namespace sqc
