#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "src/common/channel_id_hash.h"
#include "src/exchange/exchange_adapter.h"

namespace sqc {

struct ChannelInfo {
  uint32_t id;
  std::string exchange;
  ChannelType type = ChannelType::Spot;
  std::string symbol;
  uint32_t depth_level = 10;
  std::string topic_prefix;  // "exchange:type:symbol", filled by Register()
};

class ChannelRegistry {
 public:
  static ChannelRegistry& Instance();

  uint32_t Register(const ChannelInfo& info);
  const ChannelInfo* Lookup(uint32_t id) const;

 private:
  ChannelRegistry() = default;
  ChannelRegistry(const ChannelRegistry&) = delete;
  ChannelRegistry& operator=(const ChannelRegistry&) = delete;

  std::unordered_map<uint32_t, ChannelInfo> channels_;
};

}  // namespace sqc
