#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "src/common/tick_data.h"

namespace sqc {

// DolphinDB storage client, per spec §4.1
// Uses TCP protocol for tableInsert and upsert operations.
class DolphinDBClient {
 public:
  DolphinDBClient() = default;

  bool Connect(const std::string& host, uint16_t port, const std::string& user,
               const std::string& password);
  void Disconnect();
  bool IsHealthy() const;

  // Batch insert via tableInsert
  bool TableInsert(const std::string& table_name, const std::vector<TickData>& batch);

  // Upsert for offline recovery with composite key [channel_id, exchange_timestamp, trade_id]
  bool Upsert(const std::string& table_name, const std::vector<TickData>& batch);

 private:
  std::string host_;
  uint16_t port_ = 0;
  bool connected_ = false;
};

}  // namespace sqc
