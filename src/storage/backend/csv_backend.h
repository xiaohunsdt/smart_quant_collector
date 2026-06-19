#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "storage/engine/csv_writer.h"
#include "storage/i_storage_backend.h"

namespace sqc {

/// CSV IStorageBackend: owns one CsvWriter per registered channel and
/// dispatches inserts by channel_id under a mutex. The per-channel CsvWriter
/// (and its tests) stay unchanged; this is the thin dispatch layer the router
/// delegates to instead of a string compare.
class CsvBackend : public IStorageBackend {
 public:
  CsvBackend() = default;
  ~CsvBackend() override = default;
  CsvBackend(const CsvBackend&) = delete;
  CsvBackend& operator=(const CsvBackend&) = delete;
  CsvBackend(CsvBackend&&) = delete;
  CsvBackend& operator=(CsvBackend&&) = delete;

  void RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                       uint32_t depth_level) override;
  void InsertTick(const TickData& tick) override;
  void InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level) override;
  void InsertBookTicker(const BookTickerEvent& event) override;
  void FlushAndClose() override;
  const char* Name() const noexcept override { return "csv"; }

  // Set before RegisterChannel (StorageRouter does this from config). Plain
  // member rather than ctor arg so the factory can default-construct.
  void SetOutputRoot(std::string root) { output_root_ = std::move(root); }

 private:
  std::string output_root_;
  std::unordered_map<uint32_t, CsvWriter> writers_;
  std::mutex mtx_;
};

}  // namespace sqc
