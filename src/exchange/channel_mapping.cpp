#include "exchange/channel_mapping.h"

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

ChannelRegistry& ChannelRegistry::Instance() {
  static ChannelRegistry instance;
  return instance;
}

uint32_t ChannelRegistry::Register(const ChannelInfo& info) {
  // Called only during single-threaded init before parser threads start.
  uint32_t id = ComputeChannelId(info.exchange, ChannelTypeName(info.type), info.symbol);

  if(channels_.count(id)) [[unlikely]] {
    LOG_CRITICAL(GetLogger(), "channel_id collision: id={} exchange={} type={} symbol={}", id, info.exchange, ChannelTypeName(info.type),
                 info.symbol);
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
