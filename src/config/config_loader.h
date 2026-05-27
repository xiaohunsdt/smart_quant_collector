#pragma once

#include <string>

#include "config_struct.h"

namespace sqc {

RootConfig LoadConfig(const std::string& path);

}  // namespace sqc
