#include "storage_router.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/orderbook/local_lob.h"

namespace sqc {

StorageRouter::StorageRouter(const StorageConfig& storage_cfg,
                             const std::vector<ExchangeConfig>& exchanges)
    : use_engine_(storage_cfg.use_engine),
      buffer_size_(storage_cfg.dolphindb.buffer_size),
      buffer_a_(storage_cfg.dolphindb.buffer_size),
      buffer_b_(storage_cfg.dolphindb.buffer_size) {
  buffer_a_.reserve(buffer_size_);
  buffer_b_.reserve(buffer_size_);

  if (use_engine_ == "dolphindb")
    dolphindb_.Connect(storage_cfg.dolphindb.host, storage_cfg.dolphindb.port,
                       storage_cfg.dolphindb.user, storage_cfg.dolphindb.password);

  mmap_.OpenOrCreate(storage_cfg.mmap.output_path);

  if (use_engine_ == "csv") {
    for (const auto& ex : exchanges) {
      if (!ex.enabled) continue;
      CsvWriter w;
      w.Open(storage_cfg.csv.output_path, ex.name);
      csv_writers_.emplace(ex.name, std::move(w));
    }
  }
}

void StorageRouter::RouteTick(const TickData& tick, std::string_view exchange) {
  if (use_engine_ == "csv") {
    auto it = csv_writers_.find(std::string(exchange));
    if (it != csv_writers_.end()) it->second.AppendTick(tick);
  } else if (use_engine_ == "mmap") {
    mmap_.AppendRecord(tick, 0);
  } else if (use_engine_ == "dolphindb") {
    auto& buf = ActiveBuffer();
    buf.push_back(tick);
    if (buf.size() >= buffer_size_) {
      FlushActiveBuffer();
      SwapBuffer();
      ActiveBuffer().clear();
    }
  }
}

void StorageRouter::RouteOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                                    uint64_t local_ts, std::string_view symbol,
                                    uint32_t depth_level, std::string_view exchange) {
  if (use_engine_ == "csv") {
    auto it = csv_writers_.find(std::string(exchange));
    if (it != csv_writers_.end())
      it->second.AppendOrderbook(lob, exchange_ts, local_ts, symbol, depth_level);
  } else if (use_engine_ == "mmap") {
    TickData stub{};
    stub.exchange_timestamp = exchange_ts;
    stub.local_timestamp = local_ts;
    std::strncpy(stub.symbol, symbol.data(), sizeof(stub.symbol) - 1);
    mmap_.AppendRecord(stub, 0);
  }
}

void StorageRouter::FlushActiveBuffer() {
  auto& buf = ActiveBuffer();
  if (buf.empty()) return;

  if (!degraded_ && dolphindb_.IsHealthy()) {
    bool ok = dolphindb_.TableInsert("trades", buf);
    if (!ok) {
      LOG_WARNING(GetLogger(), "StorageRouter: DolphinDB insert failed, degrading to mmap");
      degraded_ = true;
    }
  }

  if (degraded_) {
    for (const auto& tick : buf)
      mmap_.AppendRecord(tick, 1);
  }
}

void StorageRouter::FlushAndClose() {
  FlushActiveBuffer();
  mmap_.Sync();
  mmap_.Close();
  dolphindb_.Disconnect();
}

std::vector<TickData>& StorageRouter::ActiveBuffer() {
  return active_index_.load(std::memory_order_acquire) == 0 ? buffer_a_ : buffer_b_;
}

void StorageRouter::SwapBuffer() {
  size_t cur = active_index_.load(std::memory_order_relaxed);
  active_index_.store(cur ^ 1, std::memory_order_release);
}

}  // namespace sqc
