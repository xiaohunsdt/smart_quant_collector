#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class LocalLOB;
class OrderbookStateMachine;

class OrderbookManager {
 public:
  OrderbookManager() = default;

  void RegisterChannel(uint32_t channel_id, uint32_t depth_level);
  void OnTick(std::shared_ptr<TickData> tick);
  void OnDepthEvent(uint32_t channel_id, const DepthUpdateEvent& event);

  LocalLOB* GetLOB(uint32_t channel_id);
  OrderbookStateMachine* GetFSM(uint32_t channel_id);
  uint32_t GetDepthLevel(uint32_t channel_id) const;

 private:
  std::unordered_map<uint32_t, std::unique_ptr<LocalLOB>> lobs_;
  std::unordered_map<uint32_t, std::unique_ptr<OrderbookStateMachine>> fsms_;
  std::unordered_map<uint32_t, uint32_t> depth_levels_;
};

}  // namespace sqc
