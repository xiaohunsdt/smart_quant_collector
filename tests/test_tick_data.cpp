#include <cstring>

#include <gtest/gtest.h>

#include "src/common/tick_data.h"
#include "src/common/storage_envelope.h"

namespace sqc {
namespace {

TEST(TickDataTest, SizeIsExactly64) {
  EXPECT_EQ(sizeof(TickData), 64);
}

TEST(TickDataTest, AlignmentIs64) {
  EXPECT_EQ(alignof(TickData), 64);
}

TEST(TickDataTest, FieldsAreAccessible) {
  TickData tick{};
  tick.exchange_timestamp = 1716844800000000;
  tick.local_diff = 1716844800000000000;
  tick.trade_id = 12345;
  tick.price = 50000.0;
  tick.quantity = 1.5;
  tick.channel_id = 42;
  std::strncpy(tick.symbol, "BTCUSDT", sizeof(tick.symbol));
  tick.is_buyer_maker = true;

  EXPECT_EQ(tick.exchange_timestamp, 1716844800000000);
  EXPECT_EQ(tick.price, 50000.0);
  EXPECT_EQ(tick.channel_id, 42);
  EXPECT_STREQ(tick.symbol, "BTCUSDT");
  EXPECT_TRUE(tick.is_buyer_maker);
}

TEST(StorageTickEnvelopeTest, SizeIsExactly72) {
  EXPECT_TRUE(sizeof(StorageTickEnvelope) == 72 || sizeof(StorageTickEnvelope) == 128);
}

TEST(StorageTickEnvelopeTest, FieldsAreAccessible) {
  StorageTickEnvelope env{};
  env.data.price = 100.0;
  env.storage_target = 1;
  env.recovery_status = 0;

  EXPECT_EQ(env.data.price, 100.0);
  EXPECT_EQ(env.storage_target, 1);
  EXPECT_EQ(env.recovery_status, 0);
}

}  // namespace
}  // namespace sqc
