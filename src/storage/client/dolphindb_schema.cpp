#include "storage/client/dolphindb_schema.h"

#include <fmt/format.h>

#include <algorithm>
#include <ctime>

#include "common/logger_init.h"
#include "quill/LogMacros.h"
#include "storage/timestamp_util.h"

namespace sqc {

std::string DolphinDBSchema::BuildRangeSpec(int start_year, int end_year, const std::string& granularity) {
  // Build DolphinDB RANGE boundary specification as a date vector.
  //
  // CRITICAL: We use date(["YYYY.MM.DD",...]) vector syntax (single function call,
  // comma-separated array) to avoid DolphinDB's 1024-operator-per-expression limit.
  // Chaining 1460+ individual date()..date() calls for daily granularity would hit
  // "The number of operators in an expression is larger than the supported maximum
  //  value: 1024" — the vector form has exactly 1 operator regardless of element count.
  std::string dates;  // inner comma-separated quoted date strings

  if (granularity == "day") {
    for (int y = start_year; y <= end_year; ++y) {
      for (int m = 1; m <= 12; ++m) {
        int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) days_in_month[1] = 29;
        for (int d = 1; d <= days_in_month[m - 1]; ++d) {
          if (!dates.empty()) dates += ',';
          dates += '"' + fmt::format("{:04d}.{:02d}.{:02d}", y, m, d) + '"';
        }
      }
    }
  } else if (granularity == "month") {
    for (int y = start_year; y <= end_year; ++y) {
      for (int m = 1; m <= 12; ++m) {
        if (!dates.empty()) dates += ',';
        dates += '"' + fmt::format("{:04d}.{:02d}.01", y, m) + '"';
      }
    }
  } else {  // "year" (also default)
    for (int y = start_year; y <= end_year; ++y) {
      if (!dates.empty()) dates += ',';
      dates += '"' + fmt::format("{:04d}.01.01", y) + '"';
    }
  }

  return "date([" + dates + "])";
}

bool DolphinDBSchema::TryExtendPartitionRange(dolphindb::DBConnection& conn, const std::string& db_path, int start_year, int end_year,
                                              const std::string& granularity) {
  try {
    // Query the current max partition boundary from the first-level RANGE domain.
    auto result = conn.run(fmt::format("db = database(\"{0}\");"
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

    if (type_str != "RANGE") {
      LOG_WARNING(GetLogger(), "DolphinDB: expected RANGE partition, got '{}' — skipping auto-extend", type_str);
      return true;  // Not an error — just can't auto-extend non-RANGE databases
    }

    // Extract last (max) boundary date.
    std::string last_boundary;
    size_t last_dotdot = boundaries_str.rfind("..");
    if (last_dotdot != std::string::npos && last_dotdot + 2 < boundaries_str.size()) {
      last_boundary = boundaries_str.substr(last_dotdot + 2);
    } else {
      last_boundary = boundaries_str;
    }
    last_boundary.erase(0, last_boundary.find_first_not_of(" \t\n\r"));
    last_boundary.erase(last_boundary.find_last_not_of(" \t\n\r") + 1);

    int last_year = 0;
    if (last_boundary.size() >= 4 && std::isdigit(last_boundary[0])) {
      last_year = std::stoi(last_boundary.substr(0, 4));
    } else {
      size_t quote1 = last_boundary.find('"');
      if (quote1 != std::string::npos && quote1 + 5 < last_boundary.size()) {
        last_year = std::stoi(last_boundary.substr(quote1 + 1, 4));
      }
    }

    if (last_year < end_year) {
      std::string new_boundaries = BuildRangeSpec(std::max(last_year, start_year), end_year, granularity);
      conn.run(fmt::format("addValuePartitions(database(\"{0}\"), {1})", db_path, new_boundaries));
      LOG_INFO(GetLogger(), "DolphinDB: extended partition range to {} (granularity={})", end_year, granularity);
    } else {
      LOG_DEBUG(GetLogger(), "DolphinDB: partition range up-to-date (last boundary year={})", last_year);
    }
    return true;
  } catch (const std::exception& e) {
    // Auto-extend failure is non-fatal: data within existing range still works.
    LOG_WARNING(GetLogger(), "DolphinDB: TryExtendPartitionRange failed (non-fatal): {}", e.what());
    return false;
  }
}

bool DolphinDBSchema::InitSchema(dolphindb::DBConnection& conn, const Config& cfg) {
  const std::string& db_path = cfg.dfs_db_path;
  const std::string& granularity = cfg.partition_granularity;

  // Validate partition_granularity
  if (granularity != "day" && granularity != "month" && granularity != "year") {
    LOG_ERROR(GetLogger(), "DolphinDB: InitSchema rejected — invalid partition_granularity '{}' (use: day|month|year)", granularity);
    return false;
  }

  // Validate dfs_db_path charset.
  for (char c : db_path) {
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/' || c == '_' || c == ':' || c == '-' ||
          c == '.' || c == '~')) {
      LOG_ERROR(GetLogger(), "DolphinDB: InitSchema rejected — invalid character '{}' in dfs_db_path \"{}\"", c, db_path);
      return false;
    }
  }

  // Compute dynamic date range from the shared partition window constants.
  auto now = std::time(nullptr);
  struct tm tm_buf {};
  localtime_r(&now, &tm_buf);
  int cur_year = tm_buf.tm_year + 1900;
  int start_year = cur_year - timestamp_util::kPartitionYearBack;
  int end_year = cur_year + timestamp_util::kPartitionYearForward;

  try {
    // Step 1: Create database if not exists; if exists, auto-extend partition range.
    {
      auto result = conn.run(fmt::format("existsDatabase(\"{}\")", db_path));
      bool db_exists = result->getBool();

      if (!db_exists) {
        std::string range_spec = BuildRangeSpec(start_year, end_year, granularity);
        conn.run(fmt::format("database(\"{0}\", COMPO, ["
                             "database(\"\", RANGE, {1}),"
                             "database(\"\", HASH, [SYMBOL, {2}])"
                             "], , \"TSDB\")",
                             db_path, range_spec, cfg.hash_buckets));
        LOG_INFO(GetLogger(), "DolphinDB: created database {} (granularity={}, range={}-{})", db_path, granularity, start_year, end_year);
      } else {
        LOG_DEBUG(GetLogger(), "DolphinDB: database {} already exists", db_path);
        if (!TryExtendPartitionRange(conn, db_path, start_year, end_year, granularity)) {
          LOG_WARNING(GetLogger(), "DolphinDB: partition range extension failed — data outside existing range may be lost");
        }
      }
    }

    // Steps 2-4: create DFS tables, stream tables, subscriptions for all three.
    const TableSpec* specs[] = {&kTrades, &kOrderbook, &kBookTicker};

    // DFS partitioned tables.
    for (const TableSpec* s : specs) {
      auto result = conn.run(fmt::format("existsTable(\"{}\", \"{}\")", db_path, s->name));
      if (!result->getBool()) {
        conn.run(fmt::format("db = database(\"{0}\"); "
                             "createPartitionedTable(db, table(1:0, {1}, {2}), \"{3}\", "
                             "`trade_date`symbol, , `symbol`exchange_timestamp)",
                             db_path, s->cols, s->types, s->name));
        LOG_INFO(GetLogger(), "DolphinDB: created DFS table '{}'", s->name);
      }
    }

    // Shared stream tables (race-tolerant: another instance may have created them).
    for (const TableSpec* s : specs) {
      auto result = conn.run(fmt::format("exists(\"{}\")", s->stream_name));
      bool exists = false;
      try {
        exists = result->getBool();
      } catch (...) {  // NOLINT(bugprone-empty-catch) — benign: existence check races
      }
      if (!exists) {
        try {
          conn.run(fmt::format("share streamTable(1:0, {}, {}) as {}", s->cols, s->types, s->stream_name));
          LOG_INFO(GetLogger(), "DolphinDB: created stream table '{}'", s->stream_name);
        } catch (const std::exception& e) {
          std::string err = e.what();
          if (err.find("already") != std::string::npos || err.find("registered") != std::string::npos) {
            LOG_DEBUG(GetLogger(), "DolphinDB: stream table '{}' already registered (benign)", s->stream_name);
          } else {
            throw;
          }
        }
      }
    }

    // Subscribe stream tables to DFS (race-tolerant).
    for (const TableSpec* s : specs) {
      auto result = conn.run(fmt::format("exec count(*) from getStreamingStat().pubTables where tableName = \"{}\"", s->stream_name));
      int count = 0;
      try {
        count = result->getInt();
      } catch (...) {  // NOLINT(bugprone-empty-catch)
      }
      if (count == 0) {
        try {
          conn.run(fmt::format("subscribeTable(tableName=\"{0}\", actionName=\"persist_{1}\", "
                               "handler=loadTable(\"{2}\", \"{1}\"), "
                               "msgAsTable=true, batchSize={3}, throttle={4})",
                               s->stream_name, s->name, db_path, cfg.mtw_batch_size, cfg.mtw_throttle_sec));
          LOG_INFO(GetLogger(), "DolphinDB: subscribed {} -> DFS", s->stream_name);
        } catch (const std::exception& e) {
          std::string err = e.what();
          if (err.find("already") != std::string::npos || err.find("exist") != std::string::npos || err.find("duplicate") != std::string::npos) {
            LOG_WARNING(GetLogger(), "DolphinDB: subscribe {} skipped (already subscribed by another instance)", s->stream_name);
          } else {
            throw;
          }
        }
      }
    }

    LOG_INFO(GetLogger(), "DolphinDB: InitSchema complete (granularity={})", granularity);
    return true;
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: InitSchema exception: {}", e.what());
    return false;
  }
}

bool DolphinDBSchema::ValidateSchema(dolphindb::DBConnection& conn, const std::string& db_path) {
  try {
    const TableSpec* specs[] = {&kTrades, &kOrderbook, &kBookTicker};
    for (const TableSpec* s : specs) {
      // Stream table exists?
      {
        auto result = conn.run(fmt::format("exec count(*) from objs(true) where name = \"{}\"", s->stream_name));
        int count = 0;
        try {
          count = result->getInt();
        } catch (...) {  // NOLINT(bugprone-empty-catch)
        }
        if (count == 0) {
          LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema failed — stream table '{}' not found", s->stream_name);
          return false;
        }
      }
      // DFS table exists?
      {
        auto result = conn.run(fmt::format("existsTable(\"{}\", \"{}\")", db_path, s->name));
        if (!result->getBool()) {
          LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema failed — DFS table '{}' not found in {}", s->name, db_path);
          return false;
        }
      }
      // Subscription active? (non-fatal)
      {
        auto result = conn.run(fmt::format("exec count(*) from getStreamingStat().pubTables where tableName = \"{}\"", s->stream_name));
        int sub_count = 0;
        try {
          sub_count = result->getInt();
        } catch (...) {  // NOLINT(bugprone-empty-catch)
        }
        if (sub_count == 0) {
          LOG_WARNING(GetLogger(), "DolphinDB: ValidateSchema — no active subscription for '{}' (may be uninitialized)", s->stream_name);
        }
      }
    }
    LOG_DEBUG(GetLogger(), "DolphinDB: ValidateSchema passed");
    return true;
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: ValidateSchema exception: {}", e.what());
    return false;
  }
}

}  // namespace sqc
