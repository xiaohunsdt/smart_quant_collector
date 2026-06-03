#include <ctime>

#include "Util.h"

namespace dolphindb {

bool Util::getLocalTime(time_t t, struct tm& result) { return localtime_s(&result, &t) == 0; }

}  // namespace dolphindb
