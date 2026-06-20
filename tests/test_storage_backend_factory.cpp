#include <gtest/gtest.h>

#include <memory>

#include "src/config/config_struct.h"
#include "src/storage/i_storage_backend.h"
#include "storage/storage_backend_factory.h"

namespace sqc {
namespace {

TEST(StorageBackendFactoryTest, CsvEngineReturnsCsvBackend) {
  StorageConfig cfg;
  cfg.use_engine = "csv";
  cfg.csv.output_path = "/tmp/sqc_factory_test_csv/";
  auto backend = MakeBackend(cfg);
  ASSERT_NE(backend, nullptr);
  EXPECT_STREQ(backend->Name(), "csv");
}

TEST(StorageBackendFactoryTest, MmapEngineReturnsMmapBackend) {
  StorageConfig cfg;
  cfg.use_engine = "mmap";
  cfg.mmap.output_path = "/tmp/sqc_factory_test_mmap/";
  auto backend = MakeBackend(cfg);
  ASSERT_NE(backend, nullptr);
  EXPECT_STREQ(backend->Name(), "mmap");
}

TEST(StorageBackendFactoryTest, DolphinDBEngineReturnsDolphinDBBackend) {
  // Configure() attempts a real Connect() to a non-listening port; the backend
  // is still constructed and named correctly (Connect failure is logged, not
  // fatal). Use an unreachable port so this never depends on a live server.
  StorageConfig cfg;
  cfg.use_engine = "dolphindb";
  cfg.dolphindb.host = "127.0.0.1";
  cfg.dolphindb.port = 19999;  // nothing listening
  cfg.dolphindb.user = "admin";
  cfg.mmap.output_path = "/tmp/sqc_factory_test_ddb/";
  auto backend = MakeBackend(cfg);
  ASSERT_NE(backend, nullptr);
  EXPECT_STREQ(backend->Name(), "dolphindb");
}

TEST(StorageBackendFactoryTest, UnknownEngineFallsBackToCsv) {
  // Misconfiguration must not crash; it falls back to csv.
  StorageConfig cfg;
  cfg.use_engine = "this_engine_does_not_exist";
  cfg.csv.output_path = "/tmp/sqc_factory_test_unknown/";
  auto backend = MakeBackend(cfg);
  ASSERT_NE(backend, nullptr);
  EXPECT_STREQ(backend->Name(), "csv");
}

TEST(StorageBackendFactoryTest, EmptyEngineDefaultsToCsv) {
  StorageConfig cfg;
  cfg.use_engine = "";
  cfg.csv.output_path = "/tmp/sqc_factory_test_empty/";
  auto backend = MakeBackend(cfg);
  ASSERT_NE(backend, nullptr);
  EXPECT_STREQ(backend->Name(), "csv");
}

}  // namespace
}  // namespace sqc
