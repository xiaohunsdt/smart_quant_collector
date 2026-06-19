#pragma once

#include <memory>

#include "src/config/config_struct.h"

namespace sqc {

class IStorageBackend;

/// Build the storage backend selected by `cfg.use_engine`. Resolves the
/// engine string exactly once (at StorageRouter construction) so the hot path
/// never compares strings. Returns a CsvBackend for "csv", MmapBackend for
/// "mmap", DolphinDBBackend for "dolphindb". An unknown engine falls back to
/// csv with an error log (never returns null).
///
/// `cfg` is the existing StorageConfig from config_struct.h (read via
/// Config::Instance().storage) — no separate storage config type is needed.
std::unique_ptr<IStorageBackend> MakeBackend(const StorageConfig& cfg);

}  // namespace sqc
