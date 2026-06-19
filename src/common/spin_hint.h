#pragma once

#include <thread>

namespace sqc {

/// Issue an architecture-appropriate CPU spin-wait hint.
///
/// On x86-64 this emits `pause` (improves throughput and power under a tight
/// spin loop by reducing pipeline contention); on AArch64 it emits `yield`.
/// On any other architecture (RISC-V, POWER, ...) there is no single-instruction
/// pause, so we fall back to std::this_thread::yield() — a scheduler hint that
/// avoids the pure CPU-burning tight loop the empty-body version would produce.
/// Centralized here so every busy-wait loop (SPSC/MPSC queues, the rithmic
/// receiver, the order book spinlock) uses the identical low-level primitive.
[[gnu::always_inline]] inline void SpinHint() noexcept {
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
  __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
  __asm__ volatile("yield");
#else
  // No architectural spin hint available — yield the time slice so the core
  // isn't pegged at 100% in tight while() loops (e.g. PopBlocking, the rithmic
  // receiver, the order-book spinlock) on RISC-V/POWER/future appliances.
  std::this_thread::yield();
#endif
}

}  // namespace sqc
