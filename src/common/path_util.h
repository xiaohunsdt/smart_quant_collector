#pragma once

#include <cerrno>
#include <cstring>
#include <string>

#include <sys/stat.h>

namespace sqc {

/// Create path and all parent directories. Returns false on failure.
/// Works whether or not `path` ends in '/' — the final component is always
/// created (mkdir is idempotent, tolerating EEXIST). Callers need not
/// normalize via EnsureTrailingSlash() first.
inline bool EnsureDirExists(const std::string& path) {
  std::string cur;
  for(char ch : path) {
    cur += ch;
    if(ch == '/') {
      if(mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) {
        return false;
      }
    }
  }
  // If the path doesn't end in '/', the loop above never created the final
  // component — create it now (idempotent: EEXIST is treated as success).
  if(!path.empty() && path.back() != '/') {
    if(mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
      return false;
    }
  }
  return true;
}

/// Return `path` guaranteed to end in '/'. Single source of truth for the
/// trailing-slash normalization that was copy-pasted across csv_writer,
/// mmap_engine, and storage_router. Empty input returns "/".
inline std::string EnsureTrailingSlash(std::string path) {
  if(path.empty()) return "/";
  if(path.back() != '/') path += '/';
  return path;
}

/// Join two path components with exactly one separating '/'
/// (e.g. JoinPath("./mmap_cache", "degraded") → "./mmap_cache/degraded").
inline std::string JoinPath(std::string head, std::string_view tail) {
  if(!head.empty() && head.back() != '/') head += '/';
  head += tail;
  return head;
}

}  // namespace sqc
