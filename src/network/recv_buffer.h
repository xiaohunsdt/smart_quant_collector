#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace sqc {

// Receive buffer with simdjson padding, per spec §3.1
class RecvBuffer {
 public:
  static constexpr size_t kPadding = 64;  // simdjson::SIMDJSON_PADDING

  explicit RecvBuffer(size_t capacity = 1024 * 1024);

  char* data();
  const char* data() const;
  size_t size() const;
  size_t capacity() const;
  void resize(size_t n);

  // Returns size + padding — the readable range simdjson requires
  size_t padded_size() const;

 private:
  std::vector<char> buffer_;
  size_t data_size_ = 0;
};

}  // namespace sqc
