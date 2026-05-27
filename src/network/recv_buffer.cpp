#include "recv_buffer.h"

#include <algorithm>
#include <cstring>

namespace sqc {

RecvBuffer::RecvBuffer(size_t capacity) {
  buffer_.resize(capacity + kPadding, 0);
}

char* RecvBuffer::data() { return buffer_.data(); }
const char* RecvBuffer::data() const { return buffer_.data(); }
size_t RecvBuffer::size() const { return data_size_; }
size_t RecvBuffer::capacity() const { return buffer_.size() - kPadding; }

void RecvBuffer::resize(size_t n) {
  if (n > capacity()) {
    buffer_.resize(n + kPadding, 0);
  }
  data_size_ = n;
  // Zero out padding area for simdjson safety
  std::memset(buffer_.data() + data_size_, 0, kPadding);
}

size_t RecvBuffer::padded_size() const {
  return data_size_ + kPadding;
}

}  // namespace sqc
