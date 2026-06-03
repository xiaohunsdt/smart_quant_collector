#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "src/common/tick_data.h"
#include "src/orderbook/orderbook_event.h"
#include "src/storage/csv_writer.h"

namespace sqc {
namespace {

class TempDir {
 public:
  explicit TempDir(const std::string& prefix) {
    path_ = "/tmp/" + prefix + "_XXXXXX";
    const char* p = mkdtemp(path_.data());
    (void)p;
  }

  ~TempDir() {
    std::string cmd = "rm -rf " + path_;
    std::system(cmd.c_str());
  }

  const std::string& path() const { return path_; }
  bool IsValid() const { return !path_.empty(); }

 private:
  std::string path_;
};

TEST(CsvWriterTest, OpenValidDirectory) {
  TempDir dir("csv_test_open");
  ASSERT_TRUE(dir.IsValid());

  CsvWriter writer;
  EXPECT_TRUE(writer.Open(dir.path(), "test_exchange", "spot", "BTCUSDT"));
  writer.Close();
}

TEST(CsvWriterTest, EnsureDirExistsFailure) {
  CsvWriter writer;
  EXPECT_FALSE(writer.Open("/proc/readonly_nonexistent/foo", "ex", "spot", "BTCUSDT"));
}

TEST(CsvWriterTest, AppendTickWritesCorrectFormat) {
  TempDir dir("csv_test_tick");
  ASSERT_TRUE(dir.IsValid());

  CsvWriter writer;
  ASSERT_TRUE(writer.Open(dir.path(), "ex", "spot", "BTCUSDT"));

  TickData tick{};
  tick.exchange_timestamp = 1717200000000000ULL;
  tick.local_diff = 12345;
  tick.price = 50000.5;
  tick.quantity = 0.123;
  tick.channel_id = 1;
  std::strncpy(tick.symbol, "BTCUSDT", sizeof(tick.symbol) - 1);
  tick.is_buyer_maker = true;

  writer.AppendTick(tick);
  writer.Close();

  std::string file_path = dir.path() + "/ex/spot/BTCUSDT/trades_2024-06-01.csv";
  std::ifstream file(file_path);
  ASSERT_TRUE(file.is_open());

  std::string header, data;
  std::getline(file, header);
  std::getline(file, data);

  EXPECT_EQ(header, "exchange_timestamp,local_diff,price,quantity,direction");
  EXPECT_EQ(data, "1717200000000000,12345,50000.5,0.123,-1");
}

TEST(CsvWriterTest, AppendBookTickerWritesCorrectFormat) {
  TempDir dir("csv_test_bt");
  ASSERT_TRUE(dir.IsValid());

  CsvWriter writer;
  ASSERT_TRUE(writer.Open(dir.path(), "ex", "spot", "BTCUSDT"));

  BookTickerEvent event{};
  event.exchange_timestamp = 1717200000000000ULL;
  event.local_diff = 100;
  std::strncpy(event.symbol, "BTCUSDT", sizeof(event.symbol) - 1);
  event.best_bid_price = 50000.0;
  event.best_bid_qty = 1.5;
  event.best_ask_price = 50001.0;
  event.best_ask_qty = 2.0;

  writer.AppendBookTicker(event);
  writer.Close();

  std::string file_path = dir.path() + "/ex/spot/BTCUSDT/bookticker_2024-06-01.csv";
  std::ifstream file(file_path);
  ASSERT_TRUE(file.is_open());

  std::string header, data;
  std::getline(file, header);
  std::getline(file, data);

  EXPECT_EQ(header,
            "exchange_timestamp,local_diff,symbol,best_bid_price,"
            "best_bid_qty,best_ask_price,best_ask_qty");
  EXPECT_EQ(data, "1717200000000000,100,BTCUSDT,50000,1.5,50001,2");
}

}  // namespace
}  // namespace sqc
