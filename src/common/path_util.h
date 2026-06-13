#pragma once

#include <cerrno>
#include <cstring>
#include <string>

#include <sys/stat.h>

namespace sqc {

/// Create path and all parent directories. Returns false on failure.
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
  return true;
}

}  // namespace sqc
