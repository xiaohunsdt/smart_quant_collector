#include "pub_message.h"

#include <cstdio>

namespace sqc {

int64_t BuildTopic(char* buf, size_t buf_size, std::string_view exchange, std::string_view channel_type, std::string_view symbol,
                   std::string_view event_type) {
  const int written =
      std::snprintf(buf, buf_size, "%.*s:%.*s:%.*s:%.*s", static_cast<int>(exchange.size()), exchange.data(), static_cast<int>(channel_type.size()),
                    channel_type.data(), static_cast<int>(symbol.size()), symbol.data(), static_cast<int>(event_type.size()), event_type.data());

  // snprintf returns the number of bytes that *would* have been written
  // if the buffer were large enough. A return value >= buf_size means
  // truncation. A negative value means an encoding error.
  if(written < 0 || static_cast<size_t>(written) >= buf_size) {
    return -1;
  }
  return static_cast<int64_t>(written);
}

}  // namespace sqc
