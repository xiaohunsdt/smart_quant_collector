#include "storage/storage_router.h"

#include "common/logger_init.h"
#include "config/config_loader.h"
#include "quill/LogMacros.h"
#include "src/exchange/exchange_adapter.h"
#include "storage/storage_backend_factory.h"

namespace sqc {

std::atomic<StorageRouter*> StorageRouter::instance_{nullptr};

StorageRouter& StorageRouter::Instance() {
  // Acquire load: see the fully-constructed object published by the Release
  // store below. Fast path after first construction is a single atomic load.
  StorageRouter* p = instance_.load(std::memory_order_acquire);
  if (p != nullptr) return *p;
  // Slow path: construct and publish. In production this is only ever entered
  // single-threaded from main(); the atomic exchange keeps it correct even if
  // two threads race here (the loser deletes its throwaway). ResetForTesting()
  // is the only other writer and is called single-threaded from test harnesses.
  auto* created = new StorageRouter();
  StorageRouter* expected = nullptr;
  if (instance_.compare_exchange_strong(expected, created, std::memory_order_acq_rel)) {
    return *created;
  }
  // Lost the race — another thread published first. Use theirs, delete ours.
  delete created;
  return *expected;
}

void StorageRouter::ResetForTesting() {
  // Test-only: destroy the current instance and clear the slot so the next
  // Instance() call rebuilds it fresh. Safe because tests call this from a
  // single thread with no Route* in flight.
  StorageRouter* p = instance_.exchange(nullptr, std::memory_order_acq_rel);
  delete p;
}

StorageRouter::StorageRouter() {
  // Read the existing StorageConfig (config_struct.h) directly via the Config
  // singleton — no separate storage config type. The factory consumes the
  // config once and builds the selected backend.
  const StorageConfig& cfg = Config::Instance().storage;
  persist_to_disk_ = cfg.persist_to_disk;
  backend_ = MakeBackend(cfg);
  LOG_INFO(GetLogger(), "StorageRouter: active backend = '{}'", backend_->Name());
}

// ============================================================
// RegisterChannel / FreezeChannels
// ============================================================

void StorageRouter::RegisterChannel(uint32_t channel_id, const ChannelInfo& info, bool persist_to_disk) {
  // Honor the storage-level and per-symbol persistence flags. This filter is
  // backend-agnostic, so it stays in the facade rather than each backend.
  if (!persist_to_disk_ || !persist_to_disk) return;
  backend_->RegisterChannel(channel_id, info.exchange, ChannelTypeName(info.type), info.symbol, info.depth_level);
}

void StorageRouter::FreezeChannels() { backend_->FreezeChannels(); }

// ============================================================
// Route* — pure delegation
// ============================================================
//
// The router no longer inspects use_engine_: the backend is polymorphic and
// already knows how to serialize/dispatch its own records (including the
// DolphinDB degrade→reconnect→fallback sequence, which lives in
// DolphinDBBackend). Each method is now a single virtual call.

void StorageRouter::RouteTick(const TickData& tick, const ChannelInfo& /*info*/) { backend_->InsertTick(tick); }

void StorageRouter::RouteOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, const ChannelInfo& info) {
  backend_->InsertOrderbook(event, local_ts, info.depth_level);
}

void StorageRouter::RouteBookTicker(const BookTickerEvent& event, const ChannelInfo& /*info*/) { backend_->InsertBookTicker(event); }

void StorageRouter::FlushAndClose() { backend_->FlushAndClose(); }

}  // namespace sqc
