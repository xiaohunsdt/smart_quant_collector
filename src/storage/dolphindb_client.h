#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "src/common/tick_data.h"
#include "src/config/secure_string.h"

// Forward declaration — full type in dolphindb::DBConnection (<DolphinDB.h>).
// Destructor must be defined in the .cpp where the full type is visible.
namespace dolphindb {
class DBConnection;
}

namespace sqc {

/// DolphinDB storage client using the official DolphinDB C++ API.
///
/// Batch insert uses tableInsert / upsert! via conn->run(sql).
/// The composite key for upsert is [channel_id, exchange_timestamp, trade_id].
class DolphinDBClient {
 public:
  DolphinDBClient();
  ~DolphinDBClient();

  // Non-copyable, non-movable (owns DBConnection handle)
  DolphinDBClient(const DolphinDBClient&) = delete;
  DolphinDBClient& operator=(const DolphinDBClient&) = delete;
  DolphinDBClient(DolphinDBClient&&) = delete;
  DolphinDBClient& operator=(DolphinDBClient&&) = delete;

  /// Connect to a DolphinDB server. Returns true on success.
  bool Connect(const std::string& host, uint16_t port, const std::string& user, const std::string& password);

  /// Disconnect from the server.
  void Disconnect();

  /// Check if the connection is alive.
  bool IsHealthy() const;

  /// Attempt to reconnect using the last-known connection parameters.
  bool Reconnect();

  /// Batch insert via tableInsert.
  /// Returns false on failure (triggers degradation in StorageRouter).
  bool TableInsert(const std::string& table_name, const std::vector<TickData>& batch);

  /// Upsert for offline recovery.
  /// Composite key: [channel_id, exchange_timestamp, trade_id].
  bool Upsert(const std::string& table_name, const std::vector<TickData>& batch);

  /// Build a SQL VALUES clause from a tick batch (public for testing).
  static std::string BuildInsertValues(const std::string& table_name, const std::vector<TickData>& batch);

  /// Build a DolphinDB upsert! function call (public for testing).
  static std::string BuildUpsertCall(const std::string& table_name, const std::vector<TickData>& batch);

 private:
  std::string host_;
  uint16_t port_ = 0;
  std::string user_;
  SecureString password_;
  bool connected_ = false;
  std::chrono::steady_clock::time_point last_health_check_;

  std::unique_ptr<dolphindb::DBConnection> conn_;
};

}  // namespace sqc
