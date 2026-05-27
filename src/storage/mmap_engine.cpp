#include "mmap_engine.h"

#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

MmapStorageEngine::MmapStorageEngine(uint64_t max_file_size)
    : max_file_size_(max_file_size) {}

MmapStorageEngine::~MmapStorageEngine() { Close(); }

bool MmapStorageEngine::OpenOrCreate(const std::string& output_path,
                                      const std::string& file_prefix) {
  output_path_ = output_path;
  file_prefix_ = file_prefix;

  // Ensure trailing slash
  if (!output_path_.empty() && output_path_.back() != '/') {
    output_path_ += '/';
  }
  if (mkdir(output_path_.c_str(), 0755) != 0 && errno != EEXIST) {
    LOG_ERROR(GetLogger(), "Failed to create mmap output dir: {}", output_path_);
    return false;
  }

  std::string fname = MakeFileName(file_sequence_);
  fd_ = open(fname.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd_ < 0) {
    LOG_ERROR(GetLogger(), "Failed to open mmap file: {}", fname);
    return false;
  }

    long page_size = sysconf(_SC_PAGESIZE);
  uint64_t mmap_size = ((max_file_size_ + page_size - 1) / page_size) * page_size;

  if (ftruncate(fd_, static_cast<off_t>(mmap_size)) != 0) {
    LOG_ERROR(GetLogger(), "Failed to truncate mmap file: {}", fname);
    close(fd_);
    fd_ = -1;
    return false;
  }

  mmap_ptr_ = static_cast<char*>(
      mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  if (mmap_ptr_ == MAP_FAILED) {
    LOG_ERROR(GetLogger(), "mmap failed for file: {}", fname);
    close(fd_);
    fd_ = -1;
    mmap_ptr_ = nullptr;
    return false;
  }

  meta_header_ = reinterpret_cast<MmapMetaHeader*>(mmap_ptr_);
  meta_header_->file_size = max_file_size_;
  meta_header_->write_offset.store(64, std::memory_order_relaxed);

  current_mapped_offset_ = 64;
  LOG_INFO(GetLogger(), "MmapStorageEngine opened: {}", fname);
  return true;
}

void MmapStorageEngine::AppendRecord(const TickData& tick, uint32_t storage_target) {
  // Boundary defense: 2GB file boundary protection per spec §4.2
  if (current_mapped_offset_ + sizeof(StorageTickEnvelope) >= max_file_size_) {
    RollNewFile();
  }

  StorageTickEnvelope envelope{};
  envelope.data = tick;
  envelope.storage_target = storage_target;
  envelope.recovery_status = 0;

  // 1. memcpy 72 bytes into mmap space
  std::memcpy(mmap_ptr_ + current_mapped_offset_, &envelope,
              sizeof(StorageTickEnvelope));

  // 2. Memory write barrier: release semantics ensure memcpy completes before
  //    write_offset becomes visible to readers
  meta_header_->write_offset.store(current_mapped_offset_ + sizeof(StorageTickEnvelope),
                                   std::memory_order_release);

  current_mapped_offset_ += sizeof(StorageTickEnvelope);
}

void MmapStorageEngine::RollNewFile() {
  Sync();
  munmap(mmap_ptr_, max_file_size_);
  close(fd_);

  file_sequence_++;
  std::string fname = MakeFileName(file_sequence_);
  fd_ = open(fname.c_str(), O_RDWR | O_CREAT, 0644);
    long page_size = sysconf(_SC_PAGESIZE);
  uint64_t mmap_size = ((max_file_size_ + page_size - 1) / page_size) * page_size;

  if (ftruncate(fd_, static_cast<off_t>(mmap_size)) != 0) {
    LOG_ERROR(GetLogger(), "Failed to truncate new mmap file: {}", fname);
    return;
  }

  mmap_ptr_ = static_cast<char*>(
      mmap(nullptr, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
  meta_header_ = reinterpret_cast<MmapMetaHeader*>(mmap_ptr_);
  meta_header_->file_size = max_file_size_;
  meta_header_->write_offset.store(64, std::memory_order_relaxed);
  current_mapped_offset_ = 64;

  LOG_INFO(GetLogger(), "MmapStorageEngine rolled to new file: {}", fname);
}

void MmapStorageEngine::Sync() {
  if (mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
    msync(mmap_ptr_, current_mapped_offset_, MS_SYNC);
  }
}

void MmapStorageEngine::Close() {
  Sync();
  if (mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
    munmap(mmap_ptr_, max_file_size_);
    mmap_ptr_ = nullptr;
  }
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

std::string MmapStorageEngine::MakeFileName(int sequence) const {
  char buf[128];
  snprintf(buf, sizeof(buf), "%s%s_%04d.bin", output_path_.c_str(),
           file_prefix_.c_str(), sequence);
  return buf;
}

}  // namespace sqc
