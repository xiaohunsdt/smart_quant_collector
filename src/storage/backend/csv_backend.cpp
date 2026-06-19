#include "storage/backend/csv_backend.h"

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

void CsvBackend::RegisterChannel(uint32_t channel_id, std::string_view exchange, std::string_view market_type, std::string_view symbol,
                                 uint32_t depth_level) {
  // market_type doubles as the channel-type directory component ("spot"/"perpetual").
  CsvWriter w;
  if (w.Open(output_root_, exchange, market_type, symbol)) {
    std::lock_guard<std::mutex> lock(mtx_);
    writers_.emplace(channel_id, std::move(w));
  } else {
    LOG_ERROR(GetLogger(), "CsvBackend: failed to open channel dir for {}/{}/{}", exchange, market_type, symbol);
  }
}

void CsvBackend::InsertTick(const TickData& tick) {
  // Map is frozen after FreezeChannels() — concurrent find is data-race-free.
  // Look up without the lock so unknown channels early-return without
  // contending, then lock only for the stateful append.
  auto it = writers_.find(tick.channel_id);
  if (it == writers_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  it->second.AppendTick(tick);
}

void CsvBackend::InsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level) {
  auto it = writers_.find(event.channel_id);
  if (it == writers_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  it->second.AppendOrderbook(event, local_ts, depth_level);
}

void CsvBackend::InsertBookTicker(const BookTickerEvent& event) {
  auto it = writers_.find(event.channel_id);
  if (it == writers_.end()) return;
  std::lock_guard<std::mutex> lock(mtx_);
  it->second.AppendBookTicker(event);
}

void CsvBackend::FlushAndClose() {
  std::lock_guard<std::mutex> lock(mtx_);
  for (auto& [k, w] : writers_) w.Close();
}

}  // namespace sqc
