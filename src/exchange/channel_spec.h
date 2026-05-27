#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sqc {

struct SymbolSpec {
  std::string name;
  bool enabled = true;
  uint32_t depth_level = 5;
  bool record_tick = true;
};

struct ChannelSpec {
  std::string exchange_name;  // "binance", "gateio"
  std::string channel_type;   // "spot", "perpetual"
  std::string ws_url;
  std::string rest_host;      // for snapshot REST API
  std::vector<SymbolSpec> symbols;
};

}  // namespace sqc
