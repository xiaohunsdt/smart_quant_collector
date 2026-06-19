#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "src/orderbook/orderbook_event.h"
#include "storage/i_storage_backend.h"
#include "storage/engine/mmap_engine.h"
#include "storage/record_layout.h"

namespace sqc {

/// Mmap IStorageBackend: owns one MmapStorageEngine per (channel_id, record
/// type) and serializes the orderbook/bookticker binary records here (single
/// source for the layout that was previously populated inline in 4 places in
/// storage_router.cpp). Also serves as the degradation fallback sink used by
/// the DolphinDB backend.
class MmapBackend : public IStorageBackend {
 public:
  MmapBackend() = default;
  ~MmapBackend() override;
  MmapBackend(const MmapBackend&) = delete;
  MmapBackend& operator=(const MmapBackend&) = delete;
  MmapBackend(MmapBackend&&) = delete;
  MmapBackend& operator=(MmapBackend&&) = delete;

  // --- IStorageBackend ---
  void RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                       uint32_t depth_level) override;
  void InsertTick(const TickData& tick) override;
  void InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level) override;
  void InsertBookTicker(const BookTickerEvent& event) override;
  void FlushAndClose() override;
  const char* Name() const noexcept override { return "mmap"; }

  void SetOutputRoot(std::string root) { output_root_ = std::move(root); }

  // --- Degradation fallback API (used by the DolphinDB backend) ---
  // Lazily create a single fallback engine under output_root_/degraded/ and
  // append records to it. Returns without writing if the fallback could not be
  // created.
  void EnsureFallback();
  void AppendFallbackTick(const TickData& tick);
  void AppendFallbackOrderbook(const DepthUpdateEvent& event, uint64_t local_ts);
  void AppendFallbackBookTicker(const BookTickerEvent& event);

 private:
  // Serialize + append an orderbook record (header + bid/ask level arrays).
  void WriteOrderbook(MmapStorageEngine& eng, const DepthUpdateEvent& event, uint64_t local_ts);
  void WriteBookTicker(MmapStorageEngine& eng, const BookTickerEvent& event);

  std::string output_root_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> tick_engines_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> ob_engines_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> bt_engines_;
  std::unique_ptr<MmapStorageEngine> fallback_;
  std::mutex mtx_;
};

}  // namespace sqc
