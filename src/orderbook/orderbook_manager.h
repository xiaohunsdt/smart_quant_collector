#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "src/common/tick_data.h"
#include "src/exchange/exchange_adapter.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class LocalLOB;
class OrderbookStateMachine;

struct ChannelSnapshotInfo {
  std::string rest_host;
  std::string symbol;
  uint32_t depth_level = 10;
  OrderbookSnapshot (*fetch_snapshot)(std::string_view, std::string_view) = nullptr;
};

class OrderbookManager {
 public:
  OrderbookManager() = default;

  void RegisterChannel(uint32_t channel_id, uint32_t depth_level, bool snapshot_mode = false);
  void SetChannelInfo(uint32_t channel_id, ChannelSnapshotInfo info);
  void OnTick(std::shared_ptr<TickData> tick);
  void OnDepthEvent(uint32_t channel_id, const DepthUpdateEvent& event);
  bool OnBookTicker(uint32_t channel_id, const BookTickerEvent& event);

  LocalLOB* GetLOB(uint32_t channel_id);
  OrderbookStateMachine* GetFSM(uint32_t channel_id);
  uint32_t GetDepthLevel(uint32_t channel_id) const;
  void BootstrapChannel(uint32_t channel_id);

 private:
  void FetchSnapshotForChannel(uint32_t channel_id);

  std::unordered_map<uint32_t, std::unique_ptr<LocalLOB>> lobs_;
  std::unordered_map<uint32_t, std::unique_ptr<OrderbookStateMachine>> fsms_;
  std::unordered_map<uint32_t, uint32_t> depth_levels_;
  std::unordered_map<uint32_t, ChannelSnapshotInfo> channel_info_;
};

}  // namespace sqc
