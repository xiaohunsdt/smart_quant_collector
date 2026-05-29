#include "orderbook_manager.h"

#include <thread>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "lockstep_fsm.h"
#include "local_lob.h"

namespace sqc {

void OrderbookManager::RegisterChannel(uint32_t channel_id, uint32_t depth_level, bool snapshot_mode) {
  if (snapshot_mode) {
    LOG_INFO(GetLogger(), "Registering channel {} with snapshot mode", channel_id);
  } else {
    LOG_INFO(GetLogger(), "Registering channel {} without snapshot mode", channel_id);
  }
  
  auto lob = std::make_unique<LocalLOB>(depth_level);
  auto fsm = std::make_unique<OrderbookStateMachine>(*lob, snapshot_mode);
  lobs_[channel_id] = std::move(lob);
  fsms_[channel_id] = std::move(fsm);
  depth_levels_[channel_id] = depth_level;
}

void OrderbookManager::SetChannelInfo(uint32_t channel_id, ChannelSnapshotInfo info) {
  channel_info_[channel_id] = info;
  auto fsm_it = fsms_.find(channel_id);
  if (fsm_it != fsms_.end()) {
    fsm_it->second->SetSnapshotFetchCb([this, channel_id]() {
      FetchSnapshotForChannel(channel_id);
    });
  }
}

void OrderbookManager::OnTick(std::shared_ptr<TickData> tick) {
  // Trade ticks do not modify LOB state; only forwarded for persistence/pub
  (void)tick;
}

void OrderbookManager::OnDepthEvent(uint32_t channel_id, const DepthUpdateEvent& event) {
  auto it = fsms_.find(channel_id);
  if (it == fsms_.end()) return;
  it->second->OnDepthEventReceived(event);
}

bool OrderbookManager::OnBookTicker(uint32_t channel_id, const BookTickerEvent& event) {
  auto lob_it = lobs_.find(channel_id);
  if (lob_it == lobs_.end()) return false;
  return lob_it->second->UpdateBestPrice(
      event.best_bid_price, event.best_bid_qty,
      event.best_ask_price, event.best_ask_qty);
}

LocalLOB* OrderbookManager::GetLOB(uint32_t channel_id) {
  auto it = lobs_.find(channel_id);
  return it != lobs_.end() ? it->second.get() : nullptr;
}

OrderbookStateMachine* OrderbookManager::GetFSM(uint32_t channel_id) {
  auto it = fsms_.find(channel_id);
  return it != fsms_.end() ? it->second.get() : nullptr;
}

uint32_t OrderbookManager::GetDepthLevel(uint32_t channel_id) const {
  auto it = depth_levels_.find(channel_id);
  return it != depth_levels_.end() ? it->second : 10;
}

void OrderbookManager::BootstrapChannel(uint32_t channel_id) {
  auto fsm_it = fsms_.find(channel_id);
  if (fsm_it == fsms_.end()) return;
  fsm_it->second->StartBootstrap();
}

void OrderbookManager::FetchSnapshotForChannel(uint32_t channel_id) {
  auto info_it = channel_info_.find(channel_id);
  auto fsm_it = fsms_.find(channel_id);
  if (info_it == channel_info_.end() || fsm_it == fsms_.end()) return;

  auto info = info_it->second;
  auto* fsm = fsm_it->second.get();

  std::thread([info, fsm, channel_id]() {
    OrderbookSnapshot snapshot;
    if (info.fetch_snapshot) {
      snapshot = info.fetch_snapshot(info.rest_host, info.symbol);
    }
    if (snapshot.lastUpdateId == 0) {
      LOG_WARNING(GetLogger(), "Snapshot fetch returned empty for channel {}, retrying",channel_id);
      return;
    }
    fsm->PostSnapshot(snapshot, snapshot.lastUpdateId);
  }).detach();
}

}  // namespace sqc