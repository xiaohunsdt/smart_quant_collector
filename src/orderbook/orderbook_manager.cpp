#include "orderbook_manager.h"

#include "local_lob.h"
#include "lockstep_fsm.h"

namespace sqc {

void OrderbookManager::RegisterChannel(uint32_t channel_id, uint32_t depth_level,
                                          bool snapshot_mode) {
  auto lob = std::make_unique<LocalLOB>(depth_level);
  auto fsm = std::make_unique<OrderbookStateMachine>(*lob, snapshot_mode);
  lobs_[channel_id] = std::move(lob);
  fsms_[channel_id] = std::move(fsm);
  depth_levels_[channel_id] = depth_level;
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

}  // namespace sqc
