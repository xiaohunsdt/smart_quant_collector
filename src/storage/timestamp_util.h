#pragma once

#include <atomic>
#include <cstdint>
#include <ctime>

namespace sqc {

/// Pure calendar / timestamp validation helpers shared by every storage
/// backend (csv / mmap / dolphindb) and the router's hot-path validation.
///
/// All logic here is timezone-independent (UTC, pure Gregorian arithmetic)
/// and zero-allocation. Extracted from DolphinDBClient so the router no
/// longer depends on a DB client class for calendar math.
namespace timestamp_util {

/// Microseconds per UTC day. Single source of truth (was duplicated in
/// csv_writer.h and dolphindb_client.cpp).
constexpr uint64_t kUsecsPerDay = 86'400'000'000ULL;

/// Sentinel: any timestamp at or beyond this point is treated as invalid
/// (year 2100). Was an uncommented magic literal at dolphindb_client.cpp:533.
constexpr uint64_t kMaxValidUsec = 4'102'444'800'000'000ULL;

/// Earliest acceptable trade_date (2000-01-01 = day 10957 since Unix epoch).
/// Was an only-comment magic int at dolphindb_client.cpp:560.
constexpr int kMinValidDate = 10957;

/// Year-axis partition window used by the DolphinDB schema builder.
/// The RANGE partition domain spans [current_year - back, current_year + fwd].
constexpr int kPartitionYearBack = 1;
constexpr int kPartitionYearForward = 5;

/// Compute days since 1970-01-01 (UTC) via pure Gregorian calendar arithmetic.
/// Matches the DolphinDB DATE int format and is timezone-independent, unlike
/// mktime/localtime. (Was a file-scope helper in dolphindb_client.cpp.)
constexpr int64_t UtcDaysFromEpoch(int year, int month, int day) {
  int64_t y = year - 1;
  int64_t days = y * 365 + y / 4 - y / 100 + y / 400 - 719162;
  constexpr int kMonthDays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  const bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  days += kMonthDays[month - 1] + (day - 1) + (leap && month > 2 ? 1 : 0);
  return days;
}

/// Convert a microsecond timestamp to DolphinDB DATE int (days since
/// 1970.01.01). Returns -1 for the zero/overflow sentinel.
inline int UsecToDateInt(uint64_t usec_since_epoch) noexcept {
  if (usec_since_epoch == 0 || usec_since_epoch > kMaxValidUsec) {
    return -1;
  }
  return static_cast<int>(usec_since_epoch / kUsecsPerDay);
}

/// Validate that a trade_date (days since epoch) falls inside the acceptable
/// window: [kMinValidDate, end of (current_year + kPartitionYearForward)].
///
/// `max_valid_date` depends only on the current year, so it changes at most
/// once per year — yet the previous implementation recomputed it (via
/// std::time() + gmtime_r()) on every call, i.e. per tick on the hot path.
/// We cache it in an atomic and only refresh on day rollover, cutting the
/// calendar recomputation from ~once per tick to once per day.
inline bool IsValidTradeDate(int trade_date) noexcept {
  if (trade_date < 0) return false;

  // Day-count since epoch from the wall clock; used both to bound the cache and
  // to decide when cur_year has rolled. std::time is a vDSO call on Linux.
  static std::atomic<int> cached_max_date{0};
  static std::atomic<int> cached_on_day{-1};

  const time_t now = std::time(nullptr);
  const int today = static_cast<int>(now / 86400);

  int day = cached_on_day.load(std::memory_order_acquire);
  if (day != today) {
    // Day changed (or first call): recompute max_valid_date for this year.
    struct tm tm_buf {};
    gmtime_r(&now, &tm_buf);
    const int cur_year = tm_buf.tm_year + 1900;
    const int max_valid_date = static_cast<int>(UtcDaysFromEpoch(cur_year + kPartitionYearForward, 1, 1)) - 1;
    cached_max_date.store(max_valid_date, std::memory_order_release);
    cached_on_day.store(today, std::memory_order_release);
  }

  const int max_valid_date = cached_max_date.load(std::memory_order_acquire);
  return trade_date >= kMinValidDate && trade_date <= max_valid_date;
}

/// True when usec_since_epoch maps to a valid DolphinDB trade_date partition
/// key. The single entry point used by the router for hot-path validation.
inline bool IsValidExchangeTimestamp(uint64_t usec_since_epoch) noexcept {
  return IsValidTradeDate(UsecToDateInt(usec_since_epoch));
}

}  // namespace timestamp_util

}  // namespace sqc
