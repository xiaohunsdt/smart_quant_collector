#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/common/tick_data.h"
#include "src/config/config_struct.h"
#include "csv_writer.h"
#include "dolphindb_client.h"
#include "mmap_engine.h"

namespace sqc {

class LocalLOB;

class StorageRouter {
 public:
  StorageRouter(const StorageConfig& storage_cfg,
                const std::vector<ExchangeConfig>& exchanges);

  void RouteTick(const TickData& tick, std::string_view exchange);

  void RouteOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                      uint64_t local_ts, std::string_view symbol,
                      uint32_t depth_level, std::string_view exchange);

  void FlushAndClose();

 private:
  void FlushActiveBuffer();

  std::string use_engine_;
  DolphinDBClient dolphindb_;
  MmapStorageEngine mmap_;
  uint32_t buffer_size_;
  bool degraded_ = false;

  std::unordered_map<std::string, CsvWriter> csv_writers_;

  std::vector<TickData> buffer_a_;
  std::vector<TickData> buffer_b_;
  std::atomic<size_t> active_index_{0};

  std::vector<TickData>& ActiveBuffer();
  void SwapBuffer();
};

}  // namespace sqc
