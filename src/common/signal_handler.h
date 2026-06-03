#pragma once

namespace sqc {

/// Thread-safe signal handling using sigwait on a dedicated thread.
///
/// Thread constraints:
///   Install(): Must be called once before creating any threads, from the
///              main thread. Blocks SIGINT/SIGTERM for all threads.
///   IsShutdownRequested(): Thread-safe (atomic read). Safe to call from
///                          any thread at any time.
///   WaitForShutdown(): Must be called from exactly one thread. Blocks
///                      until SIGINT/SIGTERM is received. Intended for
///                      the main thread's shutdown coordination.
///   Reset(): UNSAFE if any thread is concurrently calling
///            IsShutdownRequested() or if WaitForShutdown() is blocked on
///            sigwait. Only use in single-threaded test tear-down, and
///            only after joining all threads that might call
///            IsShutdownRequested().
class SignalHandler {
 public:
  static void Install();
  static bool IsShutdownRequested();
  static void WaitForShutdown();
  static void Reset();
};

}  // namespace sqc
