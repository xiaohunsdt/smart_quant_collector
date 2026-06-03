#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "csv_writer.h"
#include "mmap_engine.h"
#include "src/common/tick_data.h"
#include "src/config/config_struct.h"
#include "src/exchange/channel_mapping.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"
#ifdef SQC_WITH_DOLPHINDB
#include "dolphindb_client.h"
#endif

namespace sqc {

class StorageRouter {
 public:
  explicit StorageRouter();

  /// Register a channel for storage. Must be called during initialization
  /// (before parser threads start) for each channel that has persist_to_disk
  /// enabled.  Uses uint32_t channel_id as the map key — zero heap allocation
  /// on the hot path.
  void RegisterChannel(uint32_t channel_id, const ChannelInfo& info, bool persist_to_disk);
  void FreezeChannels();

  void RouteTick(const TickData& tick, const ChannelInfo& info);
  void RouteOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, const ChannelInfo& info);
  void RouteBookTicker(const BookTickerEvent& event, const ChannelInfo& info);
  void FlushAndClose();

 private:
  // --- Tick double-buffer (batches multiple TickData per MTW insert call) ---
  void FlushActiveBuffer();
  void FlushBuffer(std::vector<TickData>&& batch);
  std::vector<TickData>& ActiveBuffer();
  void SwapBuffer();

  // --- Degradation helpers (shared by all three data paths) ---
  bool TryReconnect();
  void EnsureFallbackMmap();
  void WriteOrderbookToMmap(const DepthUpdateEvent& event, uint64_t local_ts);
  void WriteBookTickerToMmap(const BookTickerEvent& event);

  std::string use_engine_;
#ifdef SQC_WITH_DOLPHINDB
  DolphinDBClient dolphindb_;
#endif
  uint32_t buffer_size_;
  std::atomic<bool> degraded_{false};
  std::atomic<bool> reconnecting_{false};        // CAS gate — only one thread runs Reconnect()
  std::shared_mutex dolphindb_mtx_;       // protects dolphindb_ writer access across threads

  std::string csv_output_path_;
  std::string mmap_output_path_;

  std::unordered_map<uint32_t, CsvWriter> csv_writers_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> tick_mmap_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> ob_mmap_;
  std::unordered_map<uint32_t, std::unique_ptr<MmapStorageEngine>> bt_mmap_;

  // Mutex protecting csv_writers_, tick_mmap_, ob_mmap_, bt_mmap_, and fallback_mmap_.
  mutable std::mutex storage_mtx_;

  // Lazy-created fallback mmap engine for DolphinDB degradation.
  std::unique_ptr<MmapStorageEngine> fallback_mmap_;

  // --- Tick double-buffer ---
  std::vector<TickData> buffer_a_;
  std::vector<TickData> buffer_b_;
  std::atomic<size_t> active_index_{0};
  std::mutex buffer_mtx_;
};

}  // namespace sqc
