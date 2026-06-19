#include "storage/channel_registry.h"

#include <cassert>

namespace sqc {

void StorageChannelRegistry::Register(uint32_t channel_id, std::string exchange, std::string market_type, std::string symbol,
                                      uint32_t depth_level) {
  // Registration must complete during single-threaded startup, before Freeze()
  // and before any hot-path reader. A late Register() after Freeze() would
  // mutate the map under concurrent readers — a real data race. In Debug we
  // fail fast via assert (catches programming errors immediately); in Release
  // the LOG_ERROR + early return keeps the process alive without corrupting.
  if(IsFrozen()) {
    LOG_ERROR(GetLogger(), "StorageChannelRegistry: late Register(channel_id={}, symbol={}) rejected — registry is frozen",
              channel_id, symbol);
    assert(!IsFrozen() && "StorageChannelRegistry::Register called after Freeze() — register channels during startup only");
    return;
  }
  channels_[channel_id] = ChannelMeta{std::move(exchange), std::move(market_type), std::move(symbol), depth_level};
}

}  // namespace sqc
