#pragma once

#include <string>

#include "quill/Logger.h"

namespace sqc {

void InitLogger();
quill::Logger* GetLogger();

}  // namespace sqc
