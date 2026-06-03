#pragma once

#include <string>
#include <string_view>

namespace sqc {

// DEPRECATED — Synchronous HTTPS GET request to host:443/target.
// Returns empty string on failure.
//
// WARNING: This function heap-allocates std::string, beast::flat_buffer,
// and http::response body on every call. It is NOT used by any production
// code path — kept only for ad-hoc debugging. Do NOT call from the hot path.
std::string HttpsGet(std::string_view host, std::string_view target);

}  // namespace sqc
