// One-off helper: prints total shared memory size for the 3-queue layout.
#include <iostream>

#include "src/exchange/rithmic/rithmic_shm.h"

int main() {
  const auto size = sqc::rithmic::shm_layout::kTotalShmSize;
  std::cout << size;
  return 0;
}
