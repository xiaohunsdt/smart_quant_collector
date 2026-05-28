#pragma once

#include <string>

#include "config_struct.h"

namespace sqc {

RootConfig LoadConfig(const std::string& path);

class Config {
 public:
  static void Load(const std::string& path);
  static const RootConfig& Instance();

 private:
  static RootConfig instance_;
};

}  // namespace sqc
