#include "csv_writer.h"

#include <sys/stat.h>

#include <algorithm>
#include <cstdio>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

bool CsvWriter::Open(const std::string& trade_root, std::string_view exchange,
                     std::string_view type, std::string_view symbol) {
  dir_ = trade_root;
  if (!dir_.empty() && dir_.back() != '/') dir_ += '/';
  dir_ += exchange;
  dir_ += '/';
  dir_ += type;
  dir_ += '/';
  dir_ += symbol;
  dir_ += '/';

  if (!EnsureDirExists(dir_)) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to create directory {}", dir_);
    return false;
  }

  current_date_day_ = UINT64_MAX;
  LOG_INFO(GetLogger(), "CsvWriter ready: {}", dir_);
  return true;
}

bool CsvWriter::EnsureDirExists(const std::string& path) {
  std::string cur;
  for (char ch : path) {
    cur += ch;
    if (ch == '/') mkdir(cur.c_str(), 0755);
  }
  return true;
}

std::string CsvWriter::TimestampToDate(uint64_t usec_since_epoch) {
  uint64_t days = usec_since_epoch / kUsecsPerDay;
  uint64_t z = days + 719468;
  uint64_t era = z / 146097;
  uint64_t doe = z - era * 146097;
  uint64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  uint64_t y = yoe + era * 400;
  uint64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  uint64_t mp = (5 * doy + 2) / 153;
  uint64_t d = doy - (153 * mp + 2) / 5 + 1;
  uint64_t m = mp + (mp < 10 ? 3 : -9);
  y += (m <= 2);

  const unsigned year = static_cast<unsigned>(std::min<uint64_t>(y, 9999));
  const unsigned month = static_cast<unsigned>(std::clamp(m, uint64_t{1}, uint64_t{12}));
  const unsigned day = static_cast<unsigned>(std::clamp(d, uint64_t{1}, uint64_t{31}));

  char buf[11];  // "YYYY-MM-DD" + '\0'
  std::snprintf(buf, sizeof(buf), "%04u-%02u-%02u", year, month, day);
  return buf;
}

bool CsvWriter::RotateIfNeeded(uint64_t exchange_ts) {
  if (exchange_ts == 0) exchange_ts = 1;

  uint64_t day = exchange_ts / kUsecsPerDay;
  if (day == current_date_day_ && trade_file_.is_open())
    return true;

  Close();

  current_date_ = TimestampToDate(exchange_ts);
  current_date_day_ = day;

  std::string trade_path = dir_ + "trades_" + current_date_ + ".csv";
  bool trade_is_new = (access(trade_path.c_str(), F_OK) != 0);
  trade_file_.open(trade_path, std::ios::out | std::ios::app);
  if (!trade_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", trade_path);
    return false;
  }
  if (trade_is_new)
    trade_file_ << "exchange_timestamp,local_diff,price,quantity,direction\n";

  std::string ob_path = dir_ + "orderbook_" + current_date_ + ".csv";
  bool ob_is_new = (access(ob_path.c_str(), F_OK) != 0);
  orderbook_file_.open(ob_path, std::ios::out | std::ios::app);
  if (!orderbook_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", ob_path);
    return false;
  }
  if (ob_is_new) header_written_ = false;

  std::string bt_path = dir_ + "bookticker_" + current_date_ + ".csv";
  bool bt_is_new = (access(bt_path.c_str(), F_OK) != 0);
  bookticker_file_.open(bt_path, std::ios::out | std::ios::app);
  if (!bookticker_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", bt_path);
    return false;
  }
  if (bt_is_new)
    bookticker_file_ << "exchange_timestamp,local_diff,symbol,best_bid_price,best_bid_qty,best_ask_price,best_ask_qty\n";

  return true;
}

void CsvWriter::AppendTick(const TickData& tick) {
  if (!RotateIfNeeded(tick.exchange_timestamp)) return;
  std::string row;
  row.reserve(128);
  row += std::to_string(tick.exchange_timestamp) + ',';
  row += std::to_string(tick.local_diff) + ',';
  row += std::to_string(tick.price) + ',';
  row += std::to_string(tick.quantity) + ',';
  row += std::to_string(tick.is_buyer_maker ? -1 : 1) + '\n';
  trade_file_ << row;
}

void CsvWriter::AppendOrderbook(const DepthUpdateEvent& event, uint64_t local_ts,
                                uint32_t depth_level) {

  if (!RotateIfNeeded(event.exchange_timestamp)) return;
  if (!orderbook_file_.is_open()) return;

  if (!header_written_) {
    depth_level_ = depth_level;
    header_written_ = true;
    orderbook_file_ << "exchange_timestamp,local_diff";
    for (uint32_t i = 0; i < depth_level_; ++i)
      orderbook_file_ << ",ask_price" << (i + 1) << ",ask_size" << (i + 1)
                      << ",bid_price" << (i + 1) << ",bid_size" << (i + 1);
    orderbook_file_ << "\n";
  }

  std::string row;
  row.reserve(512);
  row += std::to_string(event.exchange_timestamp) + ',' + std::to_string(local_ts);
  for (uint32_t i = 0; i < depth_level_; ++i) {
    row += ',';
    row += std::to_string(i < event.ask_count ? event.asks[i].price : 0.0);
    row += ',';
    row += std::to_string(i < event.ask_count ? event.asks[i].quantity : 0.0);
    row += ',';
    row += std::to_string(i < event.bid_count ? event.bids[i].price : 0.0);
    row += ',';
    row += std::to_string(i < event.bid_count ? event.bids[i].quantity : 0.0);
  }
  row += '\n';
  orderbook_file_ << row;
}

void CsvWriter::AppendBookTicker(const BookTickerEvent& event) {
  if (!RotateIfNeeded(event.exchange_timestamp)) return;
  if (!bookticker_file_.is_open()) return;

  std::string row;
  row.reserve(128);
  row += std::to_string(event.exchange_timestamp) + ',';
  row += std::to_string(event.local_diff) + ',';
  row += event.symbol;
  row += ',';
  row += std::to_string(event.best_bid_price) + ',';
  row += std::to_string(event.best_bid_qty) + ',';
  row += std::to_string(event.best_ask_price) + ',';
  row += std::to_string(event.best_ask_qty) + '\n';
  bookticker_file_ << row;
}

void CsvWriter::Close() {
  if (trade_file_.is_open()) { trade_file_.flush(); trade_file_.close(); }
  if (orderbook_file_.is_open()) { orderbook_file_.flush(); orderbook_file_.close(); }
  if (bookticker_file_.is_open()) { bookticker_file_.flush(); bookticker_file_.close(); }
}

}  // namespace sqc
