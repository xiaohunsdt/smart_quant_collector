#pragma once

#include <string>

#include "quill/Logger.h"

namespace sqc {

void InitLogger(const std::string& log_file_path, const std::string& log_level_str);
quill::Logger* GetLogger();

}  // namespace sqc
