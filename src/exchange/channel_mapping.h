#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace sqc {

struct ChannelInfo {
  uint32_t id;
  std::string exchange;
  std::string type;    // "spot" or "perpetual"
  std::string symbol;
};

class ChannelRegistry {
 public:
  uint32_t Register(const ChannelInfo& info);
  const ChannelInfo* Lookup(uint32_t id) const;

 private:
  std::unordered_map<uint32_t, ChannelInfo> channels_;
  uint32_t next_id_ = 1;
};

}  // namespace sqc
