#include "storage/storage_backend_factory.h"

#include <utility>

#include "common/logger_init.h"
#include "quill/LogMacros.h"
#include "storage/backend/csv_backend.h"
#include "storage/backend/dolphindb_backend.h"
#include "storage/backend/mmap_backend.h"

namespace sqc {

std::unique_ptr<IStorageBackend> MakeBackend(const StorageConfig& cfg) {
  if (cfg.use_engine == "mmap") {
    auto backend = std::make_unique<MmapBackend>();
    backend->SetOutputRoot(cfg.mmap.output_path);
    return backend;
  }

  if (cfg.use_engine == "dolphindb") {
    auto backend = std::make_unique<DolphinDBBackend>();
    backend->Configure(cfg.dolphindb.host, cfg.dolphindb.port, cfg.dolphindb.user, cfg.dolphindb.password.get(),
                       cfg.dolphindb.buffer_size, static_cast<uint64_t>(cfg.dolphindb.flush_interval_ms) * 1'000'000ULL,
                       cfg.mmap.output_path);
    return backend;
  }

  // "csv" (default) and any unknown engine. Unknown falls back to csv so the
  // process never crashes on a misconfigured use_engine; the error is logged.
  if (cfg.use_engine != "csv") {
    LOG_ERROR(GetLogger(), "storage: unknown engine '{}', falling back to csv", cfg.use_engine);
  }
  auto backend = std::make_unique<CsvBackend>();
  backend->SetOutputRoot(cfg.csv.output_path);
  return backend;
}

}  // namespace sqc
