#include "csv_writer.h"

#include <sys/stat.h>
#include <errno.h>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/orderbook/local_lob.h"

namespace sqc {

bool CsvWriter::Open(const std::string& trade_root, std::string_view exchange) {
  std::string dir = trade_root;
  if (!dir.empty() && dir.back() != '/') dir += '/';
  dir += exchange;

  // Recursive mkdir for parent directories
  std::string cur;
  for (char ch : dir) {
    cur += ch;
    if (ch == '/') mkdir(cur.c_str(), 0755);
  }
  mkdir(dir.c_str(), 0755);

  std::string trade_path = dir + "/trades.csv";
  trade_file_.open(trade_path, std::ios::out | std::ios::trunc);
  if (!trade_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", trade_path);
    return false;
  }
  trade_file_ << "exchange_timestamp,local_timestamp,price,quantity,direction,symbol\n";

  std::string ob_path = dir + "/orderbook.csv";
  orderbook_file_.open(ob_path, std::ios::out | std::ios::trunc);
  if (!orderbook_file_.is_open()) {
    LOG_ERROR(GetLogger(), "CsvWriter: failed to open {}", ob_path);
    return false;
  }
  // Header written on first AppendOrderbook call (dynamic columns per depth_level)

  LOG_INFO(GetLogger(), "CsvWriter opened: {} ({})", dir, exchange);
  return true;
}

void CsvWriter::AppendTick(const TickData& tick) {
  if (!trade_file_.is_open()) return;
  trade_file_ << tick.exchange_timestamp << ","
              << tick.local_timestamp << ","
              << tick.price << ","
              << tick.quantity << ","
              << (tick.is_buyer_maker ? -1 : 1) << ","
              << tick.symbol << "\n";
}

void CsvWriter::AppendOrderbook(const LocalLOB& lob, uint64_t exchange_ts,
                                uint64_t local_ts, std::string_view symbol,
                                uint32_t depth_level) {
  if (!orderbook_file_.is_open()) return;

  auto bids = lob.TopBids(depth_level);
  auto asks = lob.TopAsks(depth_level);

  // Write header with correct column count (idempotent via tellp check)
  if (orderbook_file_.tellp() == 0) {
    orderbook_file_ << "exchange_timestamp,local_timestamp,symbol";
    for (uint32_t i = 0; i < depth_level; ++i) {
      orderbook_file_ << ",AskPrice" << (i + 1) << ",AskSize" << (i + 1)
                      << ",BidPrice" << (i + 1) << ",BidSize" << (i + 1);
    }
    orderbook_file_ << "\n";
  }

  orderbook_file_ << exchange_ts << "," << local_ts << "," << symbol;

  for (uint32_t i = 0; i < depth_level; ++i) {
    double ap = (i < asks.size()) ? asks[i].price : 0.0;
    double aq = (i < asks.size()) ? asks[i].quantity : 0.0;
    double bp = (i < bids.size()) ? bids[i].price : 0.0;
    double bq = (i < bids.size()) ? bids[i].quantity : 0.0;
    orderbook_file_ << "," << ap << "," << aq << "," << bp << "," << bq;
  }
  orderbook_file_ << "\n";
}

void CsvWriter::Close() {
  if (trade_file_.is_open()) {
    trade_file_.flush();
    trade_file_.close();
  }
  if (orderbook_file_.is_open()) {
    orderbook_file_.flush();
    orderbook_file_.close();
  }
}

}  // namespace sqc
