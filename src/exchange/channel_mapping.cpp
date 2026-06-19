#include "exchange/channel_mapping.h"

#include <stdexcept>
#include <string>

namespace sqc {

ChannelRegistry& ChannelRegistry::Instance() {
  static ChannelRegistry instance;
  return instance;
}

uint32_t ChannelRegistry::Register(const ChannelInfo& info) {
  // Called only during single-threaded init before parser threads start.
  uint32_t id = ComputeChannelId(info.exchange, ChannelTypeName(info.type), info.symbol);

  if(auto it = channels_.find(id); it != channels_.end()) [[unlikely]] {
    const ChannelInfo& existing = it->second;
    // Idempotent re-registration of the exact same channel is harmless (e.g.
    // a reconnect path registering again). A genuine collision — same 32-bit id
    // from two different channels — would silently mix two symbols' data into
    // one routing id, so fail fast with an actionable error.
    if(existing.exchange == info.exchange && existing.type == info.type && existing.symbol == info.symbol) {
      return id;
    }
    throw std::runtime_error("channel_id collision: id=" + std::to_string(id) + " already mapped to " + existing.exchange + ":" +
                             ChannelTypeName(existing.type) + ":" + existing.symbol + ", cannot also map " + info.exchange + ":" +
                             ChannelTypeName(info.type) + ":" + info.symbol);
  }

  ChannelInfo entry = info;
  entry.id = id;
  entry.topic_prefix = entry.exchange + ":" + ChannelTypeName(entry.type) + ":" + entry.symbol;
  channels_[id] = entry;
  return id;
}

const ChannelInfo* ChannelRegistry::Lookup(uint32_t id) const {
  auto it = channels_.find(id);
  if(it == channels_.end()) return nullptr;
  return &it->second;
}

}  // namespace sqc
