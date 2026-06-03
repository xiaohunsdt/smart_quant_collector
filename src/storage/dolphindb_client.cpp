// IMPORTANT: <fmt/format.h> (Conan fmt 11.1.4) MUST be included BEFORE
// "dolphindb_client.h" to prevent an ODR violation.  ScalarImp.h (a
// transitive public header of the DolphinDB SDK) includes the SDK's
// bundled spdlog/fmt 11.0.2, which shares the same include guard
// (FMT_FORMAT_H_) as the Conan library.  If the bundled header loads
// first, its template instantiations mix with Conan fmt 11.1.4's
// compiled symbols and trigger FMT_ASSERT(false) in write_int().
#include <fmt/format.h>

#include "dolphindb_client.h"

#include <algorithm>
#include <ctime>

#include "common/logger_init.h"
#include "config/config_loader.h"
#include "quill/LogMacros.h"

namespace sqc {

DolphinDBClient::DolphinDBClient() : conn_(std::make_unique<dolphindb::DBConnection>(false, false)) {}

DolphinDBClient::~DolphinDBClient() { Disconnect(); }

bool DolphinDBClient::Connect(const std::string& host, uint16_t port, const std::string& user, const std::string& password) {
  host_ = host;
  port_ = port;
  user_ = user;
  password_ = SecureString::FromPlain(password);

  try {
    if(!conn_->connect(host, port, user, password)) {
      LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) returned false", host, port);
      return false;
    }

    conn_->login(user, password, false);
    last_health_check_ = std::chrono::steady_clock::now();
    LOG_INFO(GetLogger(), "DolphinDB: connected to {}:{} as {}", host, port, user);

    // Auto-create schema if enabled
    if(Config::Instance().storage.dolphindb.auto_init_schema) {
      if(!InitSchema()) {
        LOG_ERROR(GetLogger(), "DolphinDB: InitSchema failed");
        conn_->close();
        // conn_.reset() is safe here: Connect() only runs during startup
        // (main thread, single-threaded) or under StorageRouter's
        // dolphindb_mtx_ exclusive lock (TryReconnect → Reconnect).
        conn_.reset();
        return false;
      }
    }

    if(!InitWriters()) {
      LOG_ERROR(GetLogger(), "DolphinDB: failed to init MTW writers");
      conn_->close();
      conn_.reset();
      return false;
    }
    connected_.store(true, std::memory_order_relaxed);
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) exception: {}", host, port, e.what());
    return false;
  }
}

// ============================================================
// InitSchema — idempotent DDL for database, tables, streams, subscriptions
// ============================================================

std::string DolphinDBClient::BuildRangeSpec(int start_year, int end_year, const std::string& granularity) {
  // Build DolphinDB RANGE boundary specification as a date vector.
  //
  // CRITICAL: We use date(["YYYY.MM.DD",...]) vector syntax (single function call,
  // comma-separated array) to avoid DolphinDB's 1024-operator-per-expression limit.
  // Chaining 1460+ individual date()..date() calls for daily granularity would hit
  // "The number of operators in an expression is larger than the supported maximum
  //  value: 1024" — the vector form has exactly 1 operator regardless of element count.
  //
  // Granularity controls boundary density:
  //   "day"   → every day:     date(["2025.01.01","2025.01.02",...])
  //   "month" → 1st of month:  date(["2025.01.01","2025.02.01",...])
  //   "year"  → 1st of year:   date(["2025.01.01","2026.01.01",...])
  std::string dates;  // inner comma-separated quoted date strings

  if(granularity == "day") {
    for(int y = start_year; y <= end_year; ++y) {
      for(int m = 1; m <= 12; ++m) {
        int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        // Leap year adjustment
        if(m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) days_in_month[1] = 29;
        for(int d = 1; d <= days_in_month[m - 1]; ++d) {
          if(!dates.empty()) dates += ',';
          dates += '"' + fmt::format("{:04d}.{:02d}.{:02d}", y, m, d) + '"';
        }
      }
    }
  } else if(granularity == "month") {
    for(int y = start_year; y <= end_year; ++y) {
      for(int m = 1; m <= 12; ++m) {
        if(!dates.empty()) dates += ',';
        dates += '"' + fmt::format("{:04d}.{:02d}.01", y, m) + '"';
      }
    }
  } else {  // "year" (also default)
    for(int y = start_year; y <= end_year; ++y) {
      if(!dates.empty()) dates += ',';
      dates += '"' + fmt::format("{:04d}.01.01", y) + '"';
    }
  }

  return "date([" + dates + "])";
}

bool DolphinDBClient::TryExtendPartitionRange(int start_year, int end_year, const std::string& granularity) {
  const auto& cfg = Config::Instance().storage.dolphindb;
  const std::string& db_path = cfg.dfs_db_path;

  try {
    // Query the current max partition boundary from the first-level RANGE domain.
    // DolphinDB COMPO database: schema(db).domain is a tuple of sub-domains.
    // We access it via exec and index into sub-domains.
    // NOTE: DolphinDB uses '.' for member access; avoid 'domain' as a variable
    // name since it can conflict with the domain keyword in some contexts.
    auto result = conn_->run(fmt::format(
        "db = database(\"{0}\");"
        "compDom = exec domain from schema(db);"
        "firstDom = compDom[0].getChildren()[0];"
        "str = firstDom.getString();"
        "boundaries = firstDom.getRange().getString();"
        "dict(`type`str`boundaries, "
        "[firstDom.getType(), str, boundaries])",
        db_path));

    auto type_key = dolphindb::Util::createString("type");
    auto boundaries_key = dolphindb::Util::createString("boundaries");
    std::string type_str = result->getMember(type_key)->getString();
    std::string boundaries_str = result->getMember(boundaries_key)->getString();

    if(type_str != "RANGE") {
      LOG_WARNING(GetLogger(), "DolphinDB: expected RANGE partition, got '{}' — skipping auto-extend", type_str);
      return true;  // Not an error — just can't auto-extend non-RANGE databases
    }

    // Extract last (max) boundary date: the last segment after ".." or the last value
    // DolphinDB RANGE boundaries use ".." as separator between range markers
    std::string last_boundary;
    size_t last_dotdot = boundaries_str.rfind("..");
    if(last_dotdot != std::string::npos && last_dotdot + 2 < boundaries_str.size()) {
      last_boundary = boundaries_str.substr(last_dotdot + 2);
    } else {
      // Single value or no ".." separator; use whole string
      last_boundary = boundaries_str;
    }

    // Trim whitespace
    last_boundary.erase(0, last_boundary.find_first_not_of(" \t\n\r"));
    last_boundary.erase(last_boundary.find_last_not_of(" \t\n\r") + 1);

    // Parse the last boundary to determine its year
    // Formats: "YYYY.MM.DD" (year) or "date(\"YYYY.MM.DD\")" (day/month)
    int last_year = 0;
    if(last_boundary.size() >= 4 && std::isdigit(last_boundary[0])) {
      // Plain "YYYY.MM.DD" format
      last_year = std::stoi(last_boundary.substr(0, 4));
    } else {
      // "date(\"YYYY.MM.DD\")" format — extract year from quotes
      size_t quote1 = last_boundary.find('"');
      if(quote1 != std::string::npos && quote1 + 5 < last_boundary.size()) {
        last_year = std::stoi(last_boundary.substr(quote1 + 1, 4));
      }
    }

    if(last_year < end_year) {
      // Need to extend: add partitions from last_year to end_year
      std::string new_boundaries = BuildRangeSpec(std::max(last_year, start_year), end_year, granularity);
      conn_->run(fmt::format("addValuePartitions(database(\"{0}\"), {1})", db_path, new_boundaries));
      LOG_INFO(GetLogger(), "DolphinDB: extended partition range to {} (granularity={})", end_year, granularity);
    } else {
      LOG_DEBUG(GetLogger(), "DolphinDB: partition range up-to-date (last boundary year={})", last_year);
    }
    return true;
  } catch(const std::exception& e) {
    // Auto-extend failure is non-fatal: data within existing range still works.
    // Data outside range will fail on insert (caught by IsValidTradeDate).
    LOG_WARNING(GetLogger(), "DolphinDB: TryExtendPartitionRange failed (non-fatal): {}", e.what());
    return false;
  }
}

bool DolphinDBClient::InitSchema() {
  const auto& cfg = Config::Instance().storage.dolphindb;
  const std::string& db_path = cfg.dfs_db_path;
  int hash_buckets = cfg.hash_buckets;
  const std::string& granularity = cfg.partition_granularity;

  // Validate partition_granularity
  if(granularity != "day" && granularity != "month" && granularity != "year") {
    LOG_ERROR(GetLogger(), "DolphinDB: InitSchema rejected — invalid partition_granularity '{}' (use: day|month|year)", granularity);
    return false;
  }

  // Validate dfs_db_path: allow alphanumeric and common filesystem chars
  for(char c : db_path) {
    if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/' || c == '_' || c == ':' || c == '-' || c == '.' ||
         c == '~')) {
      LOG_ERROR(GetLogger(), "DolphinDB: InitSchema rejected — invalid character '{}' in dfs_db_path \"{}\"", c, db_path);
      return false;
    }
  }

  // Compute dynamic date range: current_year-1 .. current_year+2
  // Use localtime_r/localtime_s to avoid data race on internal static buffer
  // (InitSchema can be called concurrently from Reconnect on multiple threads).
  auto now = std::time(nullptr);
  struct tm tm_buf {};
#if defined(__linux__) || defined(__APPLE__)
  localtime_r(&now, &tm_buf);
#elif defined(_WIN32)
  localtime_s(&tm_buf, &now);
#endif
  int cur_year = tm_buf.tm_year + 1900;
  int start_year = cur_year - 1;
  int end_year = cur_year + 5;

  try {
    // Step 1: Create database if not exists; if exists, auto-extend partition range
    {
      auto result = conn_->run(fmt::format("existsDatabase(\"{}\")", db_path));
      bool db_exists = result->getBool();

      if(!db_exists) {
        std::string range_spec = BuildRangeSpec(start_year, end_year, granularity);
        conn_->run(
            fmt::format("database(\"{0}\", COMPO, ["
                        "database(\"\", RANGE, {1}),"
                        "database(\"\", HASH, [SYMBOL, {2}])"
                        "], , \"TSDB\")",
                        db_path, range_spec, hash_buckets));
        LOG_INFO(GetLogger(), "DolphinDB: created database {} (granularity={}, range={}-{})", db_path, granularity, start_year, end_year);
      } else {
        LOG_DEBUG(GetLogger(), "DolphinDB: database {} already exists", db_path);
        // Auto-extend partition range if current date extends beyond existing boundaries
        if(!TryExtendPartitionRange(start_year, end_year, granularity)) {
          LOG_WARNING(GetLogger(), "DolphinDB: partition range extension failed — data outside existing range may be lost");
        }
      }
    }

    // Column definitions
    const char* trade_cols =
        "`exchange_timestamp`local_diff`trade_id`price`quantity"
        "`direction`is_buyer_maker`symbol`exchange`market_type`trade_date";
    const char* trade_types = "[LONG,LONG,LONG,DOUBLE,DOUBLE,INT,BOOL,SYMBOL,SYMBOL,SYMBOL,DATE]";

    const char* ob_cols =
        "`exchange_timestamp`local_diff`symbol`exchange`market_type`trade_date"
        "`bid_prices`bid_sizes`ask_prices`ask_sizes";
    const char* ob_types = "[LONG,LONG,SYMBOL,SYMBOL,SYMBOL,DATE,DOUBLE[],DOUBLE[],DOUBLE[],DOUBLE[]]";

    const char* bt_cols =
        "`exchange_timestamp`local_diff`best_bid_price`best_bid_qty"
        "`best_ask_price`best_ask_qty`symbol`exchange`market_type`trade_date";
    const char* bt_types = "[LONG,LONG,DOUBLE,DOUBLE,DOUBLE,DOUBLE,SYMBOL,SYMBOL,SYMBOL,DATE]";

    // Step 2: Create DFS tables if not exists
    auto create_dfs_table = [&](const char* name, const char* cols, const char* types) {
      auto result = conn_->run(fmt::format("existsTable(\"{}\", \"{}\")", db_path, name));
      if(!result->getBool()) {
        conn_->run(
            fmt::format("db = database(\"{0}\"); "
                        "createPartitionedTable(db, table(1:0, {1}, {2}), \"{3}\", "
                        "`trade_date`symbol, , `symbol`exchange_timestamp)",
                        db_path, cols, types, name));
        LOG_INFO(GetLogger(), "DolphinDB: created DFS table '{}'", name);
      }
    };
    create_dfs_table("trades", trade_cols, trade_types);
    create_dfs_table("orderbook", ob_cols, ob_types);
    create_dfs_table("bookticker", bt_cols, bt_types);

    // Step 3: Create stream tables if not exists
    auto create_stream = [&](const char* name, const char* cols, const char* types) {
      auto result = conn_->run(fmt::format("exists(\"{}\")", name));
      bool exists = false;
      try {
        exists = result->getBool();
      } catch(...) {
      }

      if(!exists) {
        try {
          conn_->run(fmt::format("share streamTable(1:0, {}, {}) as {}", cols, types, name));
          LOG_INFO(GetLogger(), "DolphinDB: created stream table '{}'", name);
        } catch(const std::exception& e) {
          std::string err = e.what();
          // Race: another instance (or prior connection) already registered this stream.
          if(err.find("already") != std::string::npos || err.find("registered") != std::string::npos) {
            LOG_DEBUG(GetLogger(), "DolphinDB: stream table '{}' already registered (benign)", name);
          } else {
            throw;
          }
        }
      }
    };
    create_stream("trades_stream", trade_cols, trade_types);
    create_stream("orderbook_stream", ob_cols, ob_types);
    create_stream("bookticker_stream", bt_cols, bt_types);

    // Step 4: Subscribe stream tables to DFS (if not already subscribed)
    // try-catch protects against multi-instance race: another collector may
    // have subscribed between our existence check and this subscribeTable call.
    auto subscribe_if_needed = [&](const char* stream_name, const char* dfs_table) {
      auto result =
          conn_->run(fmt::format("exec count(*) from getStreamingStat().pubTables "
                                 "where tableName = \"{}\"",
                                 stream_name));
      int count = 0;
      try {
        count = result->getInt();
      } catch(...) {
      }

      if(count == 0) {
        try {
          conn_->run(
              fmt::format("subscribeTable(tableName=\"{0}\", actionName=\"persist_{1}\", "
                          "handler=loadTable(\"{2}\", \"{1}\"), "
                          "msgAsTable=true, batchSize={3}, throttle={4})",
                          stream_name, dfs_table, db_path, cfg.mtw_batch_size, cfg.mtw_throttle_sec));
          LOG_INFO(GetLogger(), "DolphinDB: subscribed {} -> DFS", stream_name);
        } catch(const std::exception& e) {
          std::string err = e.what();
          // Race: another instance already subscribed. Treat as success.
          // TODO: Replace fragile substring matching with DolphinDB error codes
          // once the API exposes structured error codes. Current approach may
          // produce false positives on unrelated error messages.
          if(err.find("already") != std::string::npos || err.find("exist") != std::string::npos ||
             err.find("duplicate") != std::string::npos) {
            LOG_WARNING(GetLogger(), "DolphinDB: subscribe {} skipped (already subscribed by another instance)", stream_name);
          } else {
            throw;  // Re-throw unexpected errors
          }
        }
      }
    };

    subscribe_if_needed("trades_stream", "trades");
    subscribe_if_needed("orderbook_stream", "orderbook");
    subscribe_if_needed("bookticker_stream", "bookticker");

    LOG_INFO(GetLogger(), "DolphinDB: InitSchema complete (granularity={})", granularity);
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: InitSchema exception: {}", e.what());
    return false;
  }
}

// ============================================================
// InitWriters / DestroyWriters
// ============================================================

bool DolphinDBClient::InitWriters() {
  const auto& cfg = Config::Instance().storage.dolphindb;
  try {
    // password_.get() returns const std::string&; MTW copies it into internal pswd_ member.
    const auto& pw = password_.get();
    trades_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "trades_stream", false, false, nullptr,
                                                                           cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count, "trade_date",
                                                                           nullptr, dolphindb::MultithreadedTableWriter::M_Append);
    ob_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "orderbook_stream", false, false, nullptr,
                                                                       cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count, "trade_date", nullptr,
                                                                       dolphindb::MultithreadedTableWriter::M_Append);
    bt_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "bookticker_stream", false, false, nullptr,
                                                                       cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count, "trade_date", nullptr,
                                                                       dolphindb::MultithreadedTableWriter::M_Append);
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW init exception: {}", e.what());
    DestroyWriters();
    return false;
  }
}

void DolphinDBClient::DestroyWriters() {
  const auto& cfg = Config::Instance().storage.dolphindb;
  auto drain = [&cfg](auto& writer, const char* name) {
    if(!writer) return;
    try {
      dolphindb::MultithreadedTableWriter::Status status;
      writer->getStatus(status);
      if(status.unsentRows > 0 || status.sentRows > 0) {
        LOG_INFO(GetLogger(), "DolphinDB: draining {} — sent={}, unsent={}, failed={}", name, status.sentRows, status.unsentRows,
                 status.sendFailedRows);
      }
      writer->waitForThreadCompletion();
      // Read status again after drain
      dolphindb::MultithreadedTableWriter::Status finalStatus;
      writer->getStatus(finalStatus);
      if(finalStatus.unsentRows > 0 || finalStatus.sendFailedRows > 0) {
        LOG_WARNING(GetLogger(), "DolphinDB: {} drain incomplete — unsent={}, failed={}", name, finalStatus.unsentRows, finalStatus.sendFailedRows);
      }
    } catch(const std::exception& e) {
      LOG_WARNING(GetLogger(), "DolphinDB: drain {} exception: {}", name, e.what());
    }
    writer.reset();
  };

  drain(trades_writer_, "trades");
  drain(ob_writer_, "orderbook");
  drain(bt_writer_, "bookticker");
}

void DolphinDBClient::Disconnect() {
  DestroyWriters();
  if(conn_) {
    try {
      conn_->close();
    } catch(const std::exception& e) {
      LOG_WARNING(GetLogger(), "DolphinDB: close() exception: {}", e.what());
    }
  }
  connected_.store(false, std::memory_order_relaxed);
  LOG_INFO(GetLogger(), "DolphinDB: disconnected");
}

// ============================================================
// IsHealthy — real ping with throttle
// ============================================================

bool DolphinDBClient::IsHealthy() {
  bool c = connected_.load(std::memory_order_relaxed);
  if(!c) return false;

  const auto& cfg = Config::Instance().storage.dolphindb;
  if(cfg.health_check_interval_ms == 0) return c;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_health_check_).count();
  if(static_cast<uint32_t>(elapsed) < cfg.health_check_interval_ms) {
    return c;  // Use cached result within interval
  }

  // Perform lightweight ping.
  // Caller (StorageRouter) holds dolphindb_mtx_, serializing with Disconnect/Reconnect.
  try {
    auto result = conn_->run("1+1");
    if(result->isNull() || result->getInt() != 2) {
      connected_.store(false, std::memory_order_relaxed);
      return false;
    }
    last_health_check_ = now;
    return true;
  } catch(...) {
    connected_.store(false, std::memory_order_relaxed);
    return false;
  }
}

bool DolphinDBClient::Reconnect() {
  LOG_INFO(GetLogger(), "DolphinDB: attempting reconnect to {}:{}", host_, port_);

  // Step 1: Destroy old MTW writers.
  // Caller (StorageRouter::TryReconnect) holds dolphindb_mtx_ exclusive,
  // so hot-path threads are blocked — no risk of stale writer pointers.
  DestroyWriters();
  // Caller (StorageRouter::TryReconnect) holds dolphindb_mtx_ exclusive lock.
  try {
    if(conn_) conn_->close();
  } catch(...) {
  }
  connected_.store(false, std::memory_order_relaxed);
  conn_ = std::make_unique<dolphindb::DBConnection>(false, false);

  try {
    const auto& pw = password_.get();
    if(!conn_->connect(host_, port_, user_, pw)) {
      LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) returned false", host_, port_);
      return false;
    }
    conn_->login(user_, pw, false);
    last_health_check_ = std::chrono::steady_clock::now();

    // Re-initialize schema (idempotent — handles server restart from scratch)
    if(Config::Instance().storage.dolphindb.auto_init_schema) {
      if(!InitSchema()) {
        LOG_WARNING(GetLogger(), "DolphinDB: InitSchema failed on reconnect");
        conn_->close();
        conn_.reset();
        return false;
      }
    }

    if(!InitWriters()) {
      LOG_WARNING(GetLogger(), "DolphinDB: failed to init MTW on reconnect");
      conn_->close();
      conn_.reset();
      return false;
    }

    connected_.store(true, std::memory_order_relaxed);
    LOG_INFO(GetLogger(), "DolphinDB: reconnected to {}:{}", host_, port_);
    return true;
  } catch(const std::exception& e) {
    LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) exception: {}", host_, port_, e.what());
    return false;
  }
}

// ============================================================
// RegisterChannel / UsecToDateInt / ValidateSchema
// ============================================================

void DolphinDBClient::RegisterChannel(uint32_t channel_id, std::string exchange, std::string market_type, std::string symbol, uint32_t depth_level) {
  channels_[channel_id] = ChannelMeta{std::move(exchange), std::move(market_type), std::move(symbol), depth_level};
}

int DolphinDBClient::UsecToDateInt(uint64_t usec_since_epoch) {
  // Sentinel: zero or overflow timestamp is invalid
  if(usec_since_epoch == 0 || usec_since_epoch > 4102444800000000ULL) {  // beyond year 2100
    return -1;
  }
  constexpr uint64_t kUsecsPerDay = 86400000000ULL;
  return static_cast<int>(usec_since_epoch / kUsecsPerDay);
}

bool DolphinDBClient::IsValidTradeDate(int trade_date) {
  // Hard bound: before Unix epoch is clearly invalid
  if(trade_date < 0) return false;

  // Soft bound: compute valid range from current year
  auto now = std::time(nullptr);
  struct tm tm_buf {};
#if defined(__linux__) || defined(__APPLE__)
  localtime_r(&now, &tm_buf);
#elif defined(_WIN32)
  localtime_s(&tm_buf, &now);
#endif
  int cur_year = tm_buf.tm_year + 1900;

  // Earliest valid: 2000-01-01 (day 10957)
  // Latest valid: cur_year + 5 years
  constexpr int kMinValidDate = 10957;  // 2000-01-01
  struct tm end_tm {};
  end_tm.tm_year = (cur_year + 5) - 1900;
  end_tm.tm_mon = 0;
  end_tm.tm_mday = 1;
  // Zero-initialize remaining fields for deterministic mktime
  end_tm.tm_hour = 0;
  end_tm.tm_min = 0;
  end_tm.tm_sec = 0;
  int max_valid_date = static_cast<int>(std::mktime(&end_tm) / 86400);

  return trade_date >= kMinValidDate && trade_date <= max_valid_date;
}

bool DolphinDBClient::ValidateSchema() {
  if(!connected_.load(std::memory_order_relaxed)) return false;

  const auto& cfg = Config::Instance().storage.dolphindb;
  const std::string& db_path = cfg.dfs_db_path;

  try {
    // Check stream tables, DFS tables, and subscriptions for all three tables
    const char* tables[] = {"trades", "orderbook", "bookticker"};
    for(const char* name : tables) {
      std::string stream_name = std::string(name) + "_stream";

      // Check stream table exists
      {
        std::string stmt = fmt::format("exec count(*) from objs(true) where name = \"{}\"", stream_name);
        auto result = conn_->run(stmt);
        int count = 0;
        try {
          count = result->getInt();
        } catch(...) {
        }
        if(count == 0) {
          LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema failed — stream table '{}' not found", stream_name);
          return false;
        }
      }

      // Check DFS table exists
      {
        auto result = conn_->run(fmt::format("existsTable(\"{}\", \"{}\")", db_path, name));
        if(!result->getBool()) {
          LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema failed — DFS table '{}' not found in {}", name, db_path);
          return false;
        }
      }

      // Check subscription is active (skip if no subscriptions expected — e.g. fresh startup)
      {
        auto result = conn_->run(fmt::format(
            "exec count(*) from getStreamingStat().pubTables where tableName = \"{}\"",
            stream_name));
        int sub_count = 0;
        try {
          sub_count = result->getInt();
        } catch(...) {
        }
        if(sub_count == 0) {
          LOG_WARNING(GetLogger(), "DolphinDB: ValidateSchema — no active subscription for '{}' (may be uninitialized)", stream_name);
          // Non-fatal: subscription may be pending InitSchema
        }
      }
    }
    LOG_DEBUG(GetLogger(), "DolphinDB: ValidateSchema passed");
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema exception: {}", e.what());
    return false;
  }
}

// ============================================================
// TableInsertTrades
// ============================================================

bool DolphinDBClient::TableInsertTrades(const std::vector<TickData>& batch) {
  if(!connected_.load(std::memory_order_relaxed) || !trades_writer_) return false;
  if(batch.empty()) return true;

  dolphindb::ErrorCodeInfo errorInfo;
  for(const auto& t : batch) {
    std::string exchange;
    std::string market_type;
    std::string symbol;
    auto it = channels_.find(t.channel_id);
    if(it != channels_.end()) {
      exchange = it->second.exchange;
      market_type = it->second.market_type;
      symbol = it->second.symbol;
    } else {
      symbol = t.symbol;
    }
    int trade_date = UsecToDateInt(t.exchange_timestamp);
    if(!IsValidTradeDate(trade_date)) {
      LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for trade symbol={} ts={}",
                trade_date, symbol, t.exchange_timestamp);
      return false;
    }
    int direction = t.is_buyer_maker ? -1 : 1;

    bool ok = trades_writer_->insert(errorInfo, static_cast<long long>(t.exchange_timestamp), static_cast<long long>(t.local_diff),
                                     static_cast<long long>(t.trade_id), t.price, t.quantity, direction, t.is_buyer_maker, symbol, exchange,
                                     market_type, trade_date);
    if(!ok) {
      LOG_ERROR(GetLogger(), "DolphinDB: MTW trades insert failed: {}", errorInfo.errorInfo);
      return false;
    }
  }
  last_health_check_ = std::chrono::steady_clock::now();
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertTrades queued {} rows", batch.size());
  return true;
}

// ============================================================
// TableInsertOrderbook — depth_level from ChannelMeta
// ============================================================

bool DolphinDBClient::TableInsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts) {
  if(!connected_.load(std::memory_order_relaxed) || !ob_writer_) return false;

  std::string exchange;
  std::string market_type;
  std::string symbol;
  uint32_t depth_level = kMaxOrderbookLevels;  // safe default

  auto it = channels_.find(event.channel_id);
  if(it != channels_.end()) {
    exchange = it->second.exchange;
    market_type = it->second.market_type;
    symbol = it->second.symbol;
    if(it->second.depth_level > 0) {
      depth_level = std::min(it->second.depth_level, static_cast<uint32_t>(kMaxOrderbookLevels));
    } else {
      LOG_WARNING(GetLogger(), "DolphinDB: depth_level=0 for channel {}, using default {}", event.channel_id, depth_level);
    }
  } else {
    LOG_WARNING(GetLogger(), "DolphinDB: unregistered channel {} in TableInsertOrderbook", event.channel_id);
    symbol = event.symbol;
  }
  int trade_date = UsecToDateInt(event.exchange_timestamp);
  if(!IsValidTradeDate(trade_date)) {
    LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for orderbook symbol={} ts={}",
              trade_date, symbol, event.exchange_timestamp);
    return false;
  }

  // Stack arrays — zero heap (max kMaxOrderbookLevels = 100 levels)
  double bid_prices_data[kMaxOrderbookLevels] = {};
  double bid_sizes_data[kMaxOrderbookLevels] = {};
  double ask_prices_data[kMaxOrderbookLevels] = {};
  double ask_sizes_data[kMaxOrderbookLevels] = {};
  for(uint32_t i = 0; i < depth_level; ++i) {
    bid_prices_data[i] = i < event.bid_count ? event.bids[i].price : 0.0;
    bid_sizes_data[i] = i < event.bid_count ? event.bids[i].quantity : 0.0;
    ask_prices_data[i] = i < event.ask_count ? event.asks[i].price : 0.0;
    ask_sizes_data[i] = i < event.ask_count ? event.asks[i].quantity : 0.0;
  }

  // Thread-local pre-allocated vectors (zero heap after first use per thread).
  // Each parser thread gets its own set — no shared state, no data race.
  thread_local dolphindb::VectorSP t_bid_prices;
  thread_local dolphindb::VectorSP t_bid_sizes;
  thread_local dolphindb::VectorSP t_ask_prices;
  thread_local dolphindb::VectorSP t_ask_sizes;
  if(t_bid_prices.isNull()) {
    t_bid_prices = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_bid_sizes = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_ask_prices = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_ask_sizes = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
  }

  // clear() preserves capacity, appendDouble is allocation-free
  t_bid_prices->clear();
  t_bid_sizes->clear();
  t_ask_prices->clear();
  t_ask_sizes->clear();

  t_bid_prices->appendDouble(bid_prices_data, static_cast<int>(depth_level));
  t_bid_sizes->appendDouble(bid_sizes_data, static_cast<int>(depth_level));
  t_ask_prices->appendDouble(ask_prices_data, static_cast<int>(depth_level));
  t_ask_sizes->appendDouble(ask_sizes_data, static_cast<int>(depth_level));

  dolphindb::ErrorCodeInfo errorInfo;
  bool ok = ob_writer_->insert(errorInfo, static_cast<long long>(event.exchange_timestamp), static_cast<long long>(local_ts), symbol, exchange,
                               market_type, trade_date, t_bid_prices, t_bid_sizes, t_ask_prices, t_ask_sizes);
  if(!ok) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW orderbook insert failed: {}", errorInfo.errorInfo);
    return false;
  }
  last_health_check_ = std::chrono::steady_clock::now();
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertOrderbook queued");
  return true;
}

// ============================================================
// TableInsertBookTicker
// ============================================================

bool DolphinDBClient::TableInsertBookTicker(const BookTickerEvent& event) {
  if(!connected_.load(std::memory_order_relaxed) || !bt_writer_) return false;

  std::string exchange;
  std::string market_type;
  std::string symbol;
  auto it = channels_.find(event.channel_id);
  if(it != channels_.end()) {
    exchange = it->second.exchange;
    market_type = it->second.market_type;
    symbol = it->second.symbol;
  } else {
    symbol = event.symbol;
  }
  int trade_date = UsecToDateInt(event.exchange_timestamp);
  if(!IsValidTradeDate(trade_date)) {
    LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for bookticker symbol={} ts={}",
              trade_date, symbol, event.exchange_timestamp);
    return false;
  }

  dolphindb::ErrorCodeInfo errorInfo;
  bool ok =
      bt_writer_->insert(errorInfo, static_cast<long long>(event.exchange_timestamp), static_cast<long long>(event.local_diff), event.best_bid_price,
                         event.best_bid_qty, event.best_ask_price, event.best_ask_qty, symbol, exchange, market_type, trade_date);
  if(!ok) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW bookticker insert failed: {}", errorInfo.errorInfo);
    return false;
  }
  last_health_check_ = std::chrono::steady_clock::now();
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertBookTicker queued");
  return true;
}

}  // namespace sqc
