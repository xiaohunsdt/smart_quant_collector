#include "dolphindb_client.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

bool DolphinDBClient::Connect(const std::string& host, uint16_t port,
                              const std::string& user,
                              [[maybe_unused]] const std::string& password) {
  host_ = host;
  port_ = port;
  // In production: use DolphinDB C++ API or TCP protocol to connect.
  // For now: stub that logs the connection intent.
  LOG_INFO(GetLogger(), "DolphinDB client connecting to {}:{} as {}", host, port,
           user);
  connected_ = true;
  return true;
}

void DolphinDBClient::Disconnect() {
  connected_ = false;
  LOG_INFO(GetLogger(), "DolphinDB client disconnected");
}

bool DolphinDBClient::IsHealthy() const { return connected_; }

bool DolphinDBClient::TableInsert(const std::string& table_name,
                                  const std::vector<TickData>& batch) {
  if (!connected_) return false;
  LOG_DEBUG(GetLogger(), "DolphinDB tableInsert {} ({} rows)", table_name,
            batch.size());
  return true;
}

bool DolphinDBClient::Upsert(const std::string& table_name,
                             const std::vector<TickData>& batch) {
  if (!connected_) return false;
  LOG_DEBUG(GetLogger(), "DolphinDB upsert {} ({} rows)", table_name,
            batch.size());
  return true;
}

}  // namespace sqc
