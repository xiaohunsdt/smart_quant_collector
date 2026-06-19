#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "src/common/tick_data.h"
#include "src/exchange/channel_mapping.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/i_storage_backend.h"

namespace sqc {

/// Public facade over the storage subsystem. Preserved verbatim for backward
/// compatibility — the 6 public methods below are the entire contract with
/// consumers (main.cpp, crypto_factory, rithmic_process_manager,
/// data_dispatcher). Internally, all work is delegated to a single
/// IStorageBackend selected once at construction by the factory, so there is
/// no per-call string dispatch anymore.
///
/// Threading contract (unchanged):
///   - Instance()/ResetForTesting() manage the singleton.
///   - RegisterChannel/FreezeChannels run single-threaded during startup.
///   - Route* run concurrently from parser threads after FreezeChannels.
///   - FlushAndClose runs after all producers have stopped.
class StorageRouter {
 public:
  static StorageRouter& Instance();
  static void ResetForTesting();  // destroys and recreates the singleton instance

  /// Register a channel for storage. Must be called during initialization
  /// (before parser threads start) for each channel that has persist_to_disk
  /// enabled. Uses uint32_t channel_id as the map key — zero heap allocation
  /// on the hot path.
  void RegisterChannel(uint32_t channel_id, const ChannelInfo& info, bool persist_to_disk);
  void FreezeChannels();

  void RouteTick(const TickData& tick, const ChannelInfo& info);
  void RouteOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, const ChannelInfo& info);
  void RouteBookTicker(const BookTickerEvent& event, const ChannelInfo& info);
  void FlushAndClose();

 private:
  StorageRouter();
  StorageRouter(const StorageRouter&) = delete;
  StorageRouter& operator=(const StorageRouter&) = delete;

  // Atomic singleton pointer. The old code used a raw check-then-new on a
  // bare pointer, which was a data race (benign only because main() touches
  // Instance() single-threaded first — but racy by construction and it also
  // leaked, never deleted on normal exit). Atomic + acquire/release makes the
  // race impossible and lets ResetForTesting() swap it safely.
  static std::atomic<StorageRouter*> instance_;

  bool persist_to_disk_ = true;

  // The single active backend. Built once by the factory; all Route* calls
  // delegate here. Replaces the previous 9-way string dispatch + embedded
  // CsvWriter/DolphinDBClient/MmapStorageEngine members.
  std::unique_ptr<IStorageBackend> backend_;
};

}  // namespace sqc
