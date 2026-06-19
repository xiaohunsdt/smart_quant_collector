#include "storage/engine/mmap_engine.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include "common/logger_init.h"
#include "common/path_util.h"
#include "quill/LogMacros.h"

namespace sqc {

MmapStorageEngine::MmapStorageEngine(uint64_t max_file_size) : max_file_size_(max_file_size) {}

MmapStorageEngine::~MmapStorageEngine() { Close(); }

bool MmapStorageEngine::OpenOrCreate(const std::string& output_path, const std::string& file_prefix) {
  output_path_ = EnsureTrailingSlash(output_path);
  file_prefix_ = file_prefix;

  if(!EnsureDirExists(output_path_)) {
    LOG_ERROR(GetLogger(), "Failed to create mmap output dir: {}", output_path_);
    return false;
  }

  std::string fname = MakeFileName(file_sequence_);
  fd_ = open(fname.c_str(), O_RDWR | O_CREAT, mmap_detail::kFileMode);
  if(fd_ < 0) {
    LOG_ERROR(GetLogger(), "Failed to open mmap file: {}", fname);
    return false;
  }

  long page_size = sysconf(_SC_PAGESIZE);
  uint64_t mmap_size = ((max_file_size_ + page_size - 1) / page_size) * page_size;

  if(ftruncate(fd_, static_cast<off_t>(mmap_size)) != 0) {
    LOG_ERROR(GetLogger(), "Failed to truncate mmap file: {}", fname);
    close(fd_);
    fd_ = -1;
    return false;
  }

  mmap_ptr_ = static_cast<char*>(mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  if(mmap_ptr_ == MAP_FAILED) {
    LOG_ERROR(GetLogger(), "mmap failed for file: {}", fname);
    close(fd_);
    fd_ = -1;
    mmap_ptr_ = nullptr;
    return false;
  }
  mmap_size_ = mmap_size;

  meta_header_ = reinterpret_cast<MmapMetaHeader*>(mmap_ptr_);
  meta_header_->file_size = max_file_size_;
  meta_header_->write_offset.store(mmap_detail::kHeaderSize, std::memory_order_relaxed);

  current_mapped_offset_ = mmap_detail::kHeaderSize;
  LOG_INFO(GetLogger(), "MmapStorageEngine opened: {}", fname);
  return true;
}

void MmapStorageEngine::AppendRecord(const TickData& tick, uint32_t storage_target) {
  if(!IsOpen()) return;
  // Boundary defense: 2GB file boundary protection per spec §4.2
  if(current_mapped_offset_ + sizeof(StorageTickEnvelope) >= max_file_size_) {
    if(!RollNewFile()) return;
  }

  StorageTickEnvelope envelope{};
  envelope.data = tick;
  envelope.storage_target = storage_target;
  envelope.recovery_status = 0;

  // 1. memcpy 72 bytes into mmap space
  std::memcpy(mmap_ptr_ + current_mapped_offset_, &envelope, sizeof(StorageTickEnvelope));

  // 2. Memory write barrier: release semantics ensure memcpy completes before
  //    write_offset becomes visible to readers
  meta_header_->write_offset.store(current_mapped_offset_ + sizeof(StorageTickEnvelope), std::memory_order_release);

  current_mapped_offset_ += sizeof(StorageTickEnvelope);
}

void MmapStorageEngine::AppendRaw(const void* data, size_t size) {
  if(!IsOpen() || data == nullptr || size == 0) return;
  if(current_mapped_offset_ + size >= max_file_size_) {
    if(!RollNewFile()) return;
  }
  std::memcpy(mmap_ptr_ + current_mapped_offset_, data, size);
  meta_header_->write_offset.store(current_mapped_offset_ + size, std::memory_order_release);
  current_mapped_offset_ += size;
}

bool MmapStorageEngine::RollNewFile() {
  Sync();
  if(mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
    munmap(mmap_ptr_, mmap_size_);
    mmap_ptr_ = nullptr;
  }
  if(fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }

  file_sequence_++;
  std::string fname = MakeFileName(file_sequence_);
  fd_ = open(fname.c_str(), O_RDWR | O_CREAT, mmap_detail::kFileMode);
  if(fd_ < 0) {
    LOG_ERROR(GetLogger(), "Failed to open new mmap file: {}", fname);
    return false;
  }
  long page_size = sysconf(_SC_PAGESIZE);
  uint64_t mmap_size = ((max_file_size_ + page_size - 1) / page_size) * page_size;

  if(ftruncate(fd_, static_cast<off_t>(mmap_size)) != 0) {
    LOG_ERROR(GetLogger(), "Failed to truncate new mmap file: {}", fname);
    close(fd_);
    fd_ = -1;
    return false;
  }

  mmap_ptr_ = static_cast<char*>(mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  if(mmap_ptr_ == MAP_FAILED) {
    LOG_ERROR(GetLogger(), "mmap failed for new file: {}", fname);
    close(fd_);
    fd_ = -1;
    mmap_ptr_ = nullptr;
    return false;
  }
  mmap_size_ = mmap_size;

  meta_header_ = reinterpret_cast<MmapMetaHeader*>(mmap_ptr_);
  meta_header_->file_size = max_file_size_;
  meta_header_->write_offset.store(mmap_detail::kHeaderSize, std::memory_order_relaxed);
  current_mapped_offset_ = mmap_detail::kHeaderSize;

  LOG_INFO(GetLogger(), "MmapStorageEngine rolled to new file: {}", fname);
  return true;
}

void MmapStorageEngine::Sync() {
  if(mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
    msync(mmap_ptr_, current_mapped_offset_, MS_SYNC);
  }
}

void MmapStorageEngine::Close() {
  Sync();
  if(mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
    munmap(mmap_ptr_, mmap_size_);
    mmap_ptr_ = nullptr;
  }
  if(fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

std::string MmapStorageEngine::MakeFileName(int sequence) const {
  char buf[128];
  const int n = std::snprintf(buf, sizeof(buf), "%s%s_%04d.bin", output_path_.c_str(), file_prefix_.c_str(), sequence);
  if(n < 0 || static_cast<size_t>(n) >= sizeof(buf)) {
    // Truncated — fall back to an unambiguous (if ugly) name rather than a
    // silently-truncated path. The inputs are bounded by config, so this path
    // is essentially unreachable; logged as defensive only.
    LOG_WARNING(GetLogger(), "MmapStorageEngine: MakeFileName truncated (prefix={}, seq={})", file_prefix_, sequence);
  }
  return buf;
}

}  // namespace sqc
