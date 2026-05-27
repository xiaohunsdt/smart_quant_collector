#pragma once

namespace sqc {

class SignalHandler {
 public:
  static void Install();
  static bool IsShutdownRequested();
  static void WaitForShutdown();
  static void Reset();
};

}  // namespace sqc
