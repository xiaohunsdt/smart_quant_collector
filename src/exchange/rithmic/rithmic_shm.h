#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "src/common/tick_data.h"
#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/orderbook/orderbook_event.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// ShmHeader — metadata placed at the start of the shared memory region.
//
// Three MpscRithmicQueue<T, C> instances follow immediately after this header.
// Both processes map the same shm region and access the queues directly.
// All atomic members are lock-free on x86_64 — safe across processes.
// ============================================================================

struct ShmHeader {
  static constexpr uint64_t kMagic = 0x52534D5153484D50ULL;  // "RSMQSHMP"

  uint64_t magic;                        // validity check
  uint64_t version;                      // schema version (increment on layout change)
  std::atomic<uint64_t> heartbeat_ns;    // child updates every ~1s with steady_clock timestamp
  std::atomic<uint32_t> child_ready;     // child sets 1 after login+subscribe complete
  std::atomic<uint32_t> parent_ready;    // parent sets 1 after receiver thread starts

  ShmHeader()
      : magic(kMagic), version(1), heartbeat_ns(0),
        child_ready(0), parent_ready(0) {}
};

static_assert(std::is_standard_layout_v<ShmHeader>,
              "ShmHeader must be standard layout for shared memory");

// ============================================================================
// ShmQueueLayout — compile-time layout for the 3 queues in shared memory.
//
// Change queue capacities here; all offsets and sizes are computed from them.
// ============================================================================
inline namespace shm_layout {

constexpr size_t kTickQueueCapacity        = 16384;
constexpr size_t kDepthQueueCapacity       = 256;
constexpr size_t kBookTickerQueueCapacity  = 1024;

using TickQueue       = MpscRithmicQueue<TickData, kTickQueueCapacity>;
using DepthQueue      = MpscRithmicQueue<DepthUpdateEvent, kDepthQueueCapacity>;
using BookTickerQueue = MpscRithmicQueue<BookTickerEvent, kBookTickerQueueCapacity>;

constexpr size_t kTickQueueOffset       = sizeof(ShmHeader);
constexpr size_t kDepthQueueOffset      = kTickQueueOffset + sizeof(TickQueue);
constexpr size_t kBookTickerQueueOffset = kDepthQueueOffset + sizeof(DepthQueue);
constexpr size_t kTotalShmSize          = kBookTickerQueueOffset + sizeof(BookTickerQueue);

}  // namespace shm_layout

// ============================================================================
// ShmSetup — RAII helper for creating/opening a shared memory region.
// ============================================================================

class ShmSetup {
 public:
  // Create mode: create new shm, set size, construct header
  ShmSetup(const char* name, size_t total_size, bool create);
  // Open-existing mode: open and validate header
  explicit ShmSetup(const char* name);

  ~ShmSetup();

  ShmSetup(const ShmSetup&) = delete;
  ShmSetup& operator=(const ShmSetup&) = delete;

  ShmHeader* header() noexcept { return static_cast<ShmHeader*>(addr_); }
  const ShmHeader* header() const noexcept { return static_cast<const ShmHeader*>(addr_); }

  // Typed queue accessors (offsets computed at compile time via shm_layout)
  TickQueue*       tick_queue()       noexcept;
  DepthQueue*      depth_queue()      noexcept;
  BookTickerQueue* book_ticker_queue() noexcept;

  const TickQueue*       tick_queue()       const noexcept;
  const DepthQueue*      depth_queue()      const noexcept;
  const BookTickerQueue* book_ticker_queue() const noexcept;

  const char* name() const noexcept { return name_.c_str(); }
  void* addr() noexcept { return addr_; }
  size_t size() const noexcept { return size_; }

 private:
  std::string name_;
  int fd_ = -1;
  void* addr_ = nullptr;
  size_t size_ = 0;
  bool created_ = false;
};

// ============================================================================
// Inline implementations
// ============================================================================

inline ShmSetup::ShmSetup(const char* name, size_t total_size, bool create)
    : name_(name), size_(total_size), created_(create) {
  if (create) {
    // Remove stale shm file from a previous crash, then create fresh
    ::shm_unlink(name);
    fd_ = ::shm_open(name, O_CREAT | O_RDWR | O_EXCL, 0600);
    if (fd_ < 0) {
      throw std::runtime_error(std::string("ShmSetup: shm_open(O_CREAT) failed for ") + name);
    }
    if (::ftruncate(fd_, static_cast<off_t>(total_size)) != 0) {
      ::close(fd_);
      ::shm_unlink(name);
      throw std::runtime_error(std::string("ShmSetup: ftruncate failed for ") + name);
    }
  } else {
    fd_ = ::shm_open(name, O_RDWR, 0600);
    if (fd_ < 0) {
      throw std::runtime_error(std::string("ShmSetup: shm_open failed for ") + name);
    }
    // Determine actual size
    struct stat st {};
    if (::fstat(fd_, &st) != 0) {
      ::close(fd_);
      throw std::runtime_error(std::string("ShmSetup: fstat failed for ") + name);
    }
    size_ = static_cast<size_t>(st.st_size);
  }

  addr_ = ::mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (addr_ == MAP_FAILED) {
    ::close(fd_);
    if (created_) ::shm_unlink(name);
    throw std::runtime_error(std::string("ShmSetup: mmap failed for ") + name);
  }

  if (create) {
    // Placement-new the header
    new (addr_) ShmHeader();
  } else {
    // Validate existing header
    auto* hdr = static_cast<ShmHeader*>(addr_);
    if (hdr->magic != ShmHeader::kMagic) {
      ::munmap(addr_, size_);
      ::close(fd_);
      throw std::runtime_error(std::string("ShmSetup: bad magic in existing shm ") + name);
    }
  }
}

inline ShmSetup::ShmSetup(const char* name)
    : ShmSetup(name, 0, false) {
  // size_ is set from fstat in the delegating constructor
}

inline ShmSetup::~ShmSetup() {
  if (addr_ && addr_ != MAP_FAILED) {
    ::munmap(addr_, size_);
  }
  if (fd_ >= 0) {
    ::close(fd_);
  }
  if (created_) {
    ::shm_unlink(name_.c_str());
  }
}

// ---- Typed queue accessors ----

inline shm_layout::TickQueue* ShmSetup::tick_queue() noexcept {
  return reinterpret_cast<shm_layout::TickQueue*>(
      static_cast<char*>(addr_) + shm_layout::kTickQueueOffset);
}

inline shm_layout::DepthQueue* ShmSetup::depth_queue() noexcept {
  return reinterpret_cast<shm_layout::DepthQueue*>(
      static_cast<char*>(addr_) + shm_layout::kDepthQueueOffset);
}

inline shm_layout::BookTickerQueue* ShmSetup::book_ticker_queue() noexcept {
  return reinterpret_cast<shm_layout::BookTickerQueue*>(
      static_cast<char*>(addr_) + shm_layout::kBookTickerQueueOffset);
}

inline const shm_layout::TickQueue* ShmSetup::tick_queue() const noexcept {
  return const_cast<ShmSetup*>(this)->tick_queue();
}

inline const shm_layout::DepthQueue* ShmSetup::depth_queue() const noexcept {
  return const_cast<ShmSetup*>(this)->depth_queue();
}

inline const shm_layout::BookTickerQueue* ShmSetup::book_ticker_queue() const noexcept {
  return const_cast<ShmSetup*>(this)->book_ticker_queue();
}

}  // namespace rithmic
}  // namespace sqc
