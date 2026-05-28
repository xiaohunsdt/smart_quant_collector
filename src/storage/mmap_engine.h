#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "src/common/storage_envelope.h"
#include "src/common/tick_data.h"

namespace sqc {

// Mmap file header placed at offset 0, per spec §4.2
struct MmapMetaHeader {
  std::atomic<uint64_t> write_offset{64};  // 64-byte header
  uint64_t file_size;
};

class MmapStorageEngine {
 public:
  // max_file_size is configurable for testing (default 2GB)
  explicit MmapStorageEngine(uint64_t max_file_size = 2UL * 1024 * 1024 * 1024);
  ~MmapStorageEngine();

  MmapStorageEngine(const MmapStorageEngine&) = delete;
  MmapStorageEngine& operator=(const MmapStorageEngine&) = delete;

  // file_prefix: "tick" or "ob" to distinguish file types
  bool OpenOrCreate(const std::string& output_path, const std::string& file_prefix);
  void AppendRecord(const TickData& tick, uint32_t storage_target);
  // Write raw bytes (for orderbook serialization, F20)
  void AppendRaw(const void* data, size_t size);
  void Sync();
  void Close();
  bool IsOpen() const { return fd_ >= 0 && mmap_ptr_ != nullptr; }

 private:
  bool RollNewFile();
  std::string MakeFileName(int sequence) const;

  uint64_t max_file_size_;
  uint64_t mmap_size_ = 0;  // actual page-aligned mmap length
  int fd_ = -1;
  char* mmap_ptr_ = nullptr;
  MmapMetaHeader* meta_header_ = nullptr;
  uint64_t current_mapped_offset_ = 64;  // after header
  std::string output_path_;
  std::string file_prefix_;
  int file_sequence_ = 0;
};

}  // namespace sqc
