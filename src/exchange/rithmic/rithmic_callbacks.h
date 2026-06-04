#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/exchange/rithmic/rithmic_types.h"

namespace RApi {
class RCallbacks;
class AdmCallbacks;
}  // namespace RApi

namespace sqc {
namespace rithmic {

constexpr int kLoginNotLoggedIn = 0;
constexpr int kLoginAwaitingResults = 1;
constexpr int kLoginFailed = 2;
constexpr int kLoginComplete = 3;

// ============================================================================
// RithmicCallbacks — bridge between RApi callbacks and MPSC event pipeline.
//
// Owns concrete RApi::RCallbacks and RApi::AdmCallbacks subclasses (defined
// in .cpp) that override the hot-path market data methods and delegate back
// here for conversion + queue push.
// ============================================================================

class RithmicCallbacks {
 public:
  RithmicCallbacks(MpscRithmicQueue<>& queue,
                   const RithmicChannelMap& channel_map,
                   SsboeConverter& converter,
                   std::atomic<int>& rep_login_status,
                   std::atomic<int>& md_login_status,
                   uint32_t max_depth_levels);
  ~RithmicCallbacks();

  RithmicCallbacks(const RithmicCallbacks&) = delete;
  RithmicCallbacks& operator=(const RithmicCallbacks&) = delete;

  RApi::RCallbacks* AsRCallbacks() noexcept { return r_callbacks_.get(); }
  RApi::AdmCallbacks* AsAdmCallbacks() noexcept { return adm_callbacks_.get(); }

  // ---- Internal conversion helpers (called from .cpp callback impls) ----

  MpscRithmicQueue<>& queue() noexcept { return queue_; }
  const RithmicChannelMap& channel_map() const noexcept { return channel_map_; }
  SsboeConverter& converter() noexcept { return converter_; }
  std::atomic<int>& rep_login_status() noexcept { return rep_login_status_; }
  std::atomic<int>& md_login_status() noexcept { return md_login_status_; }
  uint32_t max_depth_levels() const noexcept { return max_depth_levels_; }

 private:
  MpscRithmicQueue<>& queue_;
  const RithmicChannelMap& channel_map_;
  SsboeConverter& converter_;
  std::atomic<int>& rep_login_status_;
  std::atomic<int>& md_login_status_;
  uint32_t max_depth_levels_;

  std::unique_ptr<RApi::RCallbacks> r_callbacks_;
  std::unique_ptr<RApi::AdmCallbacks> adm_callbacks_;
};

}  // namespace rithmic
}  // namespace sqc
