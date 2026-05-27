#pragma once

#include <cstdint>
#include <vector>

namespace sqc {

void PinToCore(uint32_t core_id);
std::vector<uint32_t> GetAvailableCores();

}  // namespace sqc
