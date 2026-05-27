#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class LocalLOB;

// CSV writer: trades.csv (tick) + orderbook.csv (LOBSTER snapshot format)
// Each exchange gets its own subdirectory.
class CsvWriter {
 public:
  CsvWriter() = default;

  // Open both files under trade_root/{exchange}/
  bool Open(const std::string& trade_root, std::string_view exchange);

  // Tick trade row: exchange_ts, local_ts, price, qty, direction, symbol
  void AppendTick(const TickData& tick);

  // Orderbook LOBSTER snapshot row: exchange_ts, local_ts,
  //   AskP1,AskS1,BidP1,BidS1, AskP2,AskS2,BidP2,BidS2, ...
  void AppendOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                       uint64_t local_ts, std::string_view symbol,
                       uint32_t depth_level);

  void Close();

 private:
  std::ofstream trade_file_;
  std::ofstream orderbook_file_;
};

}  // namespace sqc
