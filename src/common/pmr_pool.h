#pragma once

#include <memory_resource>

namespace sqc {

// Per-thread PMR pool, single-threaded (unsynchronized), per spec §3.1
class PmrPoolManager {
 public:
  PmrPoolManager()
      : pool_(std::pmr::pool_options{.max_blocks_per_chunk = 20000,
                                     .largest_required_pool_block = 256}),
        allocator_(&pool_) {}

  std::pmr::polymorphic_allocator<char>& allocator() { return allocator_; }
  std::pmr::unsynchronized_pool_resource& resource() { return pool_; }

 private:
  std::pmr::unsynchronized_pool_resource pool_;
  std::pmr::polymorphic_allocator<char> allocator_;
};

}  // namespace sqc
