#include "csv_writer.h"

#include <sys/stat.h>
#include <cstdio>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/orderbook/local_lob.h"

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

  current_date_day_ = UINT64_MAX;  // force rotation on first write
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
  // Howard Hinnant civil_from_days, zero-allocation
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

  char buf[11];
  std::snprintf(buf, sizeof(buf), "%04llu-%02llu-%02llu",
                static_cast<unsigned long long>(y),
                static_cast<unsigned long long>(m),
                static_cast<unsigned long long>(d));
  return buf;
}

bool CsvWriter::RotateIfNeeded(uint64_t exchange_ts) {
  if (exchange_ts == 0) exchange_ts = 1;

  uint64_t day = exchange_ts / kUsecsPerDay;
  if (day == current_date_day_ && trade_file_.is_open())
    return true;  // fast path: same day, no rotation

  Close();

  current_date_ = TimestampToDate(exchange_ts);
  current_date_day_ = day;

  // Open trade file (append mode, write header if new)
  std::string trade_path = dir_ + "trades_" + current_date_ + ".csv";
  bool trade_is_new = (access(trade_path.c_str(), F_OK) != 0);
  trade_file_.open(trade_path, std::ios::out | std::ios::app);
  if (!trade_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", trade_path);
    return false;
  }
  if (trade_is_new)
    trade_file_ << "exchange_timestamp,local_timestamp,price,quantity,direction\n";

  // Open orderbook file
  std::string ob_path = dir_ + "orderbook_" + current_date_ + ".csv";
  bool ob_is_new = (access(ob_path.c_str(), F_OK) != 0);
  orderbook_file_.open(ob_path, std::ios::out | std::ios::app);
  if (!orderbook_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", ob_path);
    return false;
  }
  if (ob_is_new) depth_level_ = 0;  // trigger header on first orderbook write

  return true;
}

void CsvWriter::AppendTick(const TickData& tick) {
  if (!RotateIfNeeded(tick.exchange_timestamp)) return;
  trade_file_ << tick.exchange_timestamp << ","
              << tick.local_timestamp << ","
              << tick.price << ","
              << tick.quantity << ","
              << (tick.is_buyer_maker ? -1 : 1)
              << "\n";
}

void CsvWriter::AppendOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                                uint64_t local_ts,
                                [[maybe_unused]] std::string_view symbol,
                                uint32_t depth_level) {
  if (!RotateIfNeeded(exchange_ts)) return;
  if (!orderbook_file_.is_open()) return;

  if (depth_level_ == 0) {
    depth_level_ = depth_level;
    orderbook_file_ << "exchange_timestamp,local_timestamp";
    for (uint32_t i = 0; i < depth_level_; ++i)
      orderbook_file_ << ",ask_price" << (i + 1) << ",ask_size" << (i + 1)
                      << ",bid_price" << (i + 1) << ",bid_size" << (i + 1);
    orderbook_file_ << "\n";
  }

  PriceLevel bids[kMaxOrderbookLevels];
  PriceLevel asks[kMaxOrderbookLevels];
  uint32_t bid_count = lob.TopBids(bids, depth_level_);
  uint32_t ask_count = lob.TopAsks(asks, depth_level_);

  orderbook_file_ << exchange_ts << "," << local_ts;
  for (uint32_t i = 0; i < depth_level_; ++i) {
    orderbook_file_ << ","
                    << (i < ask_count ? asks[i].price : 0.0) << ","
                    << (i < ask_count ? asks[i].quantity : 0.0) << ","
                    << (i < bid_count ? bids[i].price : 0.0) << ","
                    << (i < bid_count ? bids[i].quantity : 0.0);
  }
  orderbook_file_ << "\n";
}

void CsvWriter::Close() {
  if (trade_file_.is_open()) { trade_file_.flush(); trade_file_.close(); }
  if (orderbook_file_.is_open()) { orderbook_file_.flush(); orderbook_file_.close(); }
}

}  // namespace sqc
