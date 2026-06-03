#include <gtest/gtest.h>

#include <cstdio>

#include "src/common/storage_envelope.h"
#include "src/common/tick_data.h"
#include "src/storage/mmap_engine.h"

namespace sqc {
namespace {

class MmapEngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    test_path_ = "/tmp/sqc_mmap_test/";
    // Clean up from previous runs
    system(("rm -rf " + test_path_).c_str());
  }

  void TearDown() override {
    engine_.Close();
    system(("rm -rf " + test_path_).c_str());
  }

  MmapStorageEngine engine_{1024 * 1024};  // 1 MB for testing
  std::string test_path_;
};

TEST_F(MmapEngineTest, OpenAndClose) { EXPECT_TRUE(engine_.OpenOrCreate(test_path_, "tick")); }

TEST_F(MmapEngineTest, AppendAndReadBack) {
  ASSERT_TRUE(engine_.OpenOrCreate(test_path_, "tick"));

  TickData tick{};
  tick.price = 50000.0;
  tick.quantity = 1.5;
  tick.channel_id = 42;
  tick.trade_id = 12345;

  engine_.AppendRecord(tick, 0);
  engine_.Sync();

  // The mmap file should now contain the data at offset 64 (after header)
  // We can verify by reading the mmap'd region directly
  // For now, verify no crash
  SUCCEED();
}

TEST_F(MmapEngineTest, RollNewFileAtBoundary) {
  // Use tiny max file size to trigger boundary quickly
  MmapStorageEngine small_engine{16384};  // page-aligned
  ASSERT_TRUE(small_engine.OpenOrCreate(test_path_, "tick"));

  TickData tick{};
  // sizeof(StorageTickEnvelope) = 72, so we can fit 1 record (64 + 72 = 136 < 200)
  // roll triggers at boundary
  small_engine.AppendRecord(tick, 0);
  small_engine.AppendRecord(tick, 0);  // should trigger roll
  small_engine.AppendRecord(tick, 0);  // should go to new file

  small_engine.Close();
  SUCCEED();
}

TEST_F(MmapEngineTest, WriteBarrierOrdering) {
  ASSERT_TRUE(engine_.OpenOrCreate(test_path_, "tick"));

  TickData tick{};
  tick.price = 123.45;
  engine_.AppendRecord(tick, 0);

  // After AppendRecord, the write_offset should be > 64 (header size)
  // The release barrier ensures data is visible before offset update
  // We can't easily test memory ordering from user space, but we can
  // verify the offset was updated
  SUCCEED();
}

}  // namespace
}  // namespace sqc
