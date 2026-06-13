#pragma once

#include <DolphinDB.h>
#include <MultithreadedTableWriter.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/common/tick_data.h"
#include "src/config/secure_string.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

struct ChannelMeta {
  std::string exchange;
  std::string market_type;
  std::string symbol;
  uint32_t depth_level = 0;  // 0 = unset; populated from config.yaml per symbol
};

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

  /// Mark channel registration as complete. After this, insert methods
  /// assert that channels_ is frozen (Debug builds) to catch late registrations.
  void FreezeChannels() { channels_frozen_.store(true, std::memory_order_release); }

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

  /// Convert microsecond timestamp to DolphinDB DATE int (days since 1970.01.01).
  /// Returns -1 if timestamp is zero or overflow (sentinel for invalid data).
  static int UsecToDateInt(uint64_t usec_since_epoch);

  /// Validate trade_date is within acceptable bounds.
  /// Hard-bound: >= 0 (before 1970 is illegal).
  /// Soft-bound: within [start_year, end_year] derived from current year.
  static bool IsValidTradeDate(int trade_date);

  /// True when usec_since_epoch maps to a valid DolphinDB trade_date partition key.
  static bool IsValidExchangeTimestamp(uint64_t usec_since_epoch) {
    return IsValidTradeDate(UsecToDateInt(usec_since_epoch));
  }

 private:
  /// Create database, DFS tables, stream tables, and subscriptions
  /// if they do not already exist (idempotent). Uses DDL via conn_->run().
  /// Partition granularity is driven by config.storage.dolphindb.partition_granularity.
  bool InitSchema();

  /// If the database already exists, check whether the current date range
  /// requires extension, and call addValuePartitions if needed.
  bool TryExtendPartitionRange(int start_year, int end_year, const std::string& granularity);

  /// Build the RANGE boundary specification string for DolphinDB DDL.
  /// e.g. granularity="day" → date("2025.01.01")date("2025.01.02")...
  static std::string BuildRangeSpec(int start_year, int end_year, const std::string& granularity);

  bool InitWriters();
  void DestroyWriters(bool skip_drain = false);

  std::string host_;
  uint16_t port_ = 0;
  std::string user_;
  SecureString password_;
  std::atomic<bool> connected_{false};
  std::atomic<bool> channels_frozen_{false};  // set after all RegisterChannel calls
  std::chrono::steady_clock::time_point last_health_check_;

  // conn_ is NOT guarded by an internal mutex. All callers (StorageRouter)
  // serialize access via dolphindb_mtx_ (shared_lock for hot-path
  // inserts, unique_lock for Reconnect/Disconnect), and the startup path
  // (Connect) runs single-threaded before parser threads start.
  std::unique_ptr<dolphindb::DBConnection> conn_;
  // PRECONDITION: channels_ is populated via RegisterChannel() before
  // channels_frozen_ is set to true. After that, insert methods read
  // channels_ concurrently from parser threads without locking.
  std::unordered_map<uint32_t, ChannelMeta> channels_;

  std::unique_ptr<dolphindb::MultithreadedTableWriter> trades_writer_;
  std::unique_ptr<dolphindb::MultithreadedTableWriter> ob_writer_;
  std::unique_ptr<dolphindb::MultithreadedTableWriter> bt_writer_;
};

}  // namespace sqc
