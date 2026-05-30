#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {

class LocalLOB;

// Daily-rotating CSV writer. One instance per (exchange, type, symbol) tuple.
// Files: {root}/{exchange}/{type}/{symbol}/trades_yyyy-mm-dd.csv
//        {root}/{exchange}/{type}/{symbol}/orderbook_yyyy-mm-dd.csv
//        {root}/{exchange}/{type}/{symbol}/bookticker_yyyy-mm-dd.csv
class CsvWriter {
 public:
  CsvWriter() = default;

  // Set base path and create directory structure. Files are opened lazily on first write.
  bool Open(const std::string& trade_root, std::string_view exchange,
            std::string_view type, std::string_view symbol);

  // Append tick row; rotates to a new daily file if needed.
  void AppendTick(const TickData& tick);

  // Append orderbook LOBSTER snapshot row; rotates if needed.
  void AppendOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                       uint64_t local_ts, std::string_view symbol,
                       uint32_t depth_level);

  // Append bookTicker row; rotates if needed.
  void AppendBookTicker(const BookTickerEvent& event);

  void Close();

 private:
  static std::string TimestampToDate(uint64_t usec_since_epoch);
  bool RotateIfNeeded(uint64_t exchange_ts);
  static bool EnsureDirExists(const std::string& path);

  std::string dir_;
  std::ofstream trade_file_;
  std::ofstream orderbook_file_;
  std::ofstream bookticker_file_;
  std::string current_date_;
  uint64_t current_date_day_ = UINT64_MAX;
  uint32_t depth_level_ = 0;
  bool header_written_ = false;  // decoupled from depth_level_ to survive restart

  static constexpr uint64_t kUsecsPerDay = 86400000000ULL;
};

}  // namespace sqc
