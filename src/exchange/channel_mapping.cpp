#include "exchange/channel_mapping.h"

namespace sqc {

uint32_t ChannelRegistry::Register(const ChannelInfo& info) {
  uint32_t id = next_id_++;
  ChannelInfo entry = info;
  entry.id = id;
  channels_[id] = entry;
  return id;
}

const ChannelInfo* ChannelRegistry::Lookup(uint32_t id) const {
  auto it = channels_.find(id);
  if (it == channels_.end()) return nullptr;
  return &it->second;
}

}  // namespace sqc
