#pragma once

#include <string>
#include <string_view>

namespace sqc {

// Synchronous HTTPS GET request to host:443/target.
// Returns empty string on failure.
std::string HttpsGet(std::string_view host, std::string_view target);

}  // namespace sqc
