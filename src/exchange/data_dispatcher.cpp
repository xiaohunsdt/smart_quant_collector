#include "data_dispatcher.h"

#include <cstring>

#include "src/exchange/channel_mapping.h"
#include "src/storage/storage_router.h"

namespace sqc {

void DataDispatcher::OnTick(TickData tick) const noexcept {
  const auto* info = ChannelRegistry::Instance().Lookup(tick.channel_id);
  if (!info) return;
  const uint64_t now_ns = NowNs();
  tick.local_diff = now_ns - tick.local_diff;
  std::memset(tick.padding, 0, sizeof(tick.padding));
  StorageRouter::Instance().RouteTick(tick, *info);
  pub_worker.PublishTick(tick, shard_idx);
  WriteTelemetry(tick.local_diff);
}

void DataDispatcher::OnDepth(uint32_t channel_id, const DepthUpdateEvent& event) const noexcept {
  const auto* info = ChannelRegistry::Instance().Lookup(channel_id);
  if (!info) return;
  const uint64_t latency_ns = NowNs() - event.local_diff;
  StorageRouter::Instance().RouteOrderbook(event, latency_ns, *info);
  pub_worker.PublishDepth(event, shard_idx);
  WriteTelemetry(latency_ns);
}

void DataDispatcher::OnBookTicker(uint32_t channel_id, BookTickerEvent event) const noexcept {
  const auto* info = ChannelRegistry::Instance().Lookup(channel_id);
  if (!info) return;
  const uint64_t now_ns = NowNs();
  event.local_diff = now_ns - event.local_diff;
  StorageRouter::Instance().RouteBookTicker(event, *info);
  pub_worker.PublishBookTicker(event, shard_idx);
  WriteTelemetry(event.local_diff);
}

}  // namespace sqc
