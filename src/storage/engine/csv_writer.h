#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "storage/timestamp_util.h"

namespace sqc {

// Daily-rotating CSV writer for a single (exchange, type, symbol) channel.
// Owns the 3 file streams (trades / orderbook / bookticker) and per-day
// rotation. Thread-unsafe — callers serialize access (CsvBackend holds a map
// of these under a mutex). Preserved verbatim for test compatibility
// (tests/test_csv_writer.cpp).
class CsvWriter {
 public:
  CsvWriter() = default;
  CsvWriter(const CsvWriter&) = delete;
  CsvWriter& operator=(const CsvWriter&) = delete;
  CsvWriter(CsvWriter&&) = default;
  CsvWriter& operator=(CsvWriter&&) = default;

  bool Open(const std::string& trade_root, std::string_view exchange, std::string_view type, std::string_view symbol);

  void AppendTick(const TickData& tick);

  // Append orderbook LOBSTER snapshot row directly from depth event.
  void AppendOrderbook(const DepthUpdateEvent& event, uint64_t local_ts, uint32_t depth_level);

  void AppendBookTicker(const BookTickerEvent& event);

  void Close();

 private:
  // Format a usec timestamp as "YYYY-MM-DD" (UTC, pure Gregorian).
  static std::string TimestampToDate(uint64_t usec_since_epoch);
  bool RotateIfNeeded(uint64_t exchange_ts);

  std::string dir_;
  std::ofstream trade_file_;
  std::ofstream orderbook_file_;
  std::ofstream bookticker_file_;
  std::string current_date_;
  uint64_t current_date_day_ = UINT64_MAX;
  uint32_t depth_level_ = 0;
  bool header_written_ = false;
};

}  // namespace sqc
