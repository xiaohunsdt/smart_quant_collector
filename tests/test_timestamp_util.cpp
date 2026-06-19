#include <gtest/gtest.h>

#include <ctime>

#include "storage/timestamp_util.h"

namespace sqc {
namespace {

TEST(TimestampUtilTest, UsecToDateIntZeroReturnsNegativeOne) {
  // Zero timestamp is the invalid sentinel.
  EXPECT_EQ(timestamp_util::UsecToDateInt(0), -1);
}

TEST(TimestampUtilTest, UsecToDateIntOneDay) {
  EXPECT_EQ(timestamp_util::UsecToDateInt(timestamp_util::kUsecsPerDay), 1);
}

TEST(TimestampUtilTest, UsecToDateIntOverflowReturnsNegativeOne) {
  // Beyond the year-2100 sentinel.
  constexpr uint64_t kOverflow = timestamp_util::kMaxValidUsec + 1;
  EXPECT_EQ(timestamp_util::UsecToDateInt(kOverflow), -1);
}

TEST(TimestampUtilTest, UsecToDateIntRealisticTimestamp) {
  // 2026-06-01ish — well within the valid window, must be > day 20000.
  const uint64_t ts = 1'780'445'977'532'000ULL;
  EXPECT_GT(timestamp_util::UsecToDateInt(ts), 20000);
}

TEST(TimestampUtilTest, IsValidTradeDateNegativeFails) {
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(-1));
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(-100));
}

TEST(TimestampUtilTest, IsValidTradeDateBefore2000Fails) {
  // Day 0 = 1970-01-01, before our valid window (starts 2000-01-01 = day 10957).
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(0));
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(10000));
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(timestamp_util::kMinValidDate - 1));
}

TEST(TimestampUtilTest, IsValidTradeDateMinBoundPasses) {
  // 2000-01-01 is the earliest acceptable date.
  EXPECT_TRUE(timestamp_util::IsValidTradeDate(timestamp_util::kMinValidDate));
}

TEST(TimestampUtilTest, IsValidTradeDateCurrentDatePasses) {
  const auto now = std::time(nullptr);
  const int today = static_cast<int>(now / 86400);
  EXPECT_TRUE(timestamp_util::IsValidTradeDate(today));
}

TEST(TimestampUtilTest, IsValidTradeDateFarFutureFails) {
  // ~Jan 1, 2150 — beyond the (current_year + kPartitionYearForward) window.
  const int day_2150 = 65745;
  EXPECT_FALSE(timestamp_util::IsValidTradeDate(day_2150));
}

TEST(TimestampUtilTest, IsValidExchangeTimestampZeroFails) {
  EXPECT_FALSE(timestamp_util::IsValidExchangeTimestamp(0));
}

TEST(TimestampUtilTest, IsValidExchangeTimestampValidPasses) {
  EXPECT_TRUE(timestamp_util::IsValidExchangeTimestamp(1'717'000'000'000'000ULL));
}

TEST(TimestampUtilTest, UtcDaysFromEpochKnownAnchor) {
  // 1970-01-01 is day 0.
  EXPECT_EQ(timestamp_util::UtcDaysFromEpoch(1970, 1, 1), 0);
  // 2000-01-01 is day 10957 (the kMinValidDate constant).
  EXPECT_EQ(timestamp_util::UtcDaysFromEpoch(2000, 1, 1), timestamp_util::kMinValidDate);
}

TEST(TimestampUtilTest, PartitionYearConstantsAreReasonable) {
  EXPECT_GT(timestamp_util::kPartitionYearForward, 0);
  EXPECT_GT(timestamp_util::kPartitionYearBack, 0);
}

}  // namespace
}  // namespace sqc
