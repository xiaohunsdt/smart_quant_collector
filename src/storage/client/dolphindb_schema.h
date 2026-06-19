#pragma once

#include <DolphinDB.h>

#include <cstdint>
#include <string>

namespace sqc {

/// DolphinDB schema DDL builder. Owns the database/tables/streams/subscription
/// initialization and partition-range management. Extracted from DolphinDBClient
/// (which previously inlined a 167-line InitSchema) so the DDL concern is
/// isolated and independently testable.
///
/// Stateless beyond the config it's given per call: it operates on whatever
/// DBConnection the client passes in (the client owns conn_), so this class
/// holds no connection state of its own.
class DolphinDBSchema {
 public:
  /// Column/type descriptors for each of the three tables. Named (was inline
  /// `const char*` triples inside InitSchema's body) so the schema is
  /// declarative and shareable across DFS/stream/validation paths.
  struct TableSpec {
    const char* name;          // DFS table name
    const char* stream_name;   // shared stream table name
    const char* cols;          // DolphinDB column-name vector
    const char* types;         // DolphinDB type vector
  };

  /// All three table specs, in (trades, orderbook, bookticker) order.
  static constexpr TableSpec kTrades{"trades", "trades_stream",
                                     "`exchange_timestamp`local_diff`trade_id`price`quantity"
                                     "`direction`is_buyer_maker`symbol`exchange`market_type`trade_date",
                                     "[LONG,LONG,LONG,DOUBLE,DOUBLE,INT,BOOL,SYMBOL,SYMBOL,SYMBOL,DATE]"};
  static constexpr TableSpec kOrderbook{"orderbook", "orderbook_stream",
                                        "`exchange_timestamp`local_diff`symbol`exchange`market_type`trade_date"
                                        "`bid_prices`bid_sizes`ask_prices`ask_sizes",
                                        "[LONG,LONG,SYMBOL,SYMBOL,SYMBOL,DATE,DOUBLE[],DOUBLE[],DOUBLE[],DOUBLE[]]"};
  static constexpr TableSpec kBookTicker{"bookticker", "bookticker_stream",
                                         "`exchange_timestamp`local_diff`best_bid_price`best_bid_qty"
                                         "`best_ask_price`best_ask_qty`symbol`exchange`market_type`trade_date",
                                         "[LONG,LONG,DOUBLE,DOUBLE,DOUBLE,DOUBLE,SYMBOL,SYMBOL,SYMBOL,DATE]"};

  struct Config {
    std::string dfs_db_path;
    int hash_buckets = 20;
    std::string partition_granularity = "day";  // "day" | "month" | "year"
    int mtw_batch_size = 20000;
    float mtw_throttle_sec = 1.0f;
  };

  /// Idempotently create the database, DFS tables, stream tables, and
  /// subscriptions (if they do not already exist). `conn` is the caller's
  /// connected DBConnection; the client serializes access to it. Returns false
  /// on failure (DDL exception or invalid config), true on success.
  [[nodiscard]] bool InitSchema(dolphindb::DBConnection& conn, const Config& cfg);

  /// Validate that stream tables, DFS tables, and subscriptions are present.
  /// Non-fatal if a subscription is missing (it may be pending InitSchema).
  [[nodiscard]] bool ValidateSchema(dolphindb::DBConnection& conn, const std::string& db_path);

  /// Build the RANGE boundary specification string for DolphinDB DDL.
  /// e.g. granularity="day" → date(["2025.01.01","2025.01.02",...)
  static std::string BuildRangeSpec(int start_year, int end_year, const std::string& granularity);

 private:
  /// If the database already exists, extend the partition range when the
  /// current date extends beyond the existing boundaries. Non-fatal on failure.
  [[nodiscard]] bool TryExtendPartitionRange(dolphindb::DBConnection& conn, const std::string& db_path, int start_year, int end_year,
                                             const std::string& granularity);
};

}  // namespace sqc
