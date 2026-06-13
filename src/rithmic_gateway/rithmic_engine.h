#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "RApiPlus.h"

#include "src/config/config_struct.h"
#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/exchange/rithmic/rithmic_types.h"

// Forward-declared in rithmic_callbacks.h
class MyAdmCallbacks;
class MyCallbacks;

namespace sqc {
namespace rithmic {

// ============================================================================
// RithmicEngine — Rithmic API connection + subscription.
// Matching ~/Documents/rithmic_md_saver/src/main.cpp patterns exactly.
// Uses RithmicConfig from config_struct.h for all connection parameters.
// ============================================================================

class RithmicEngine {
 public:
  using Config = RithmicConfig;

  struct SubEntry {
    bool enabled;
    std::string exchange;
    std::string ticker;
    uint32_t channel_id;
  };

  RithmicEngine(const Config& config,
                shm_layout::TickQueue& tick_queue,
                shm_layout::DepthQueue& depth_queue,
                shm_layout::BookTickerQueue& book_ticker_queue,
                const RithmicChannelMap& channel_map,
                SsboeConverter& converter,
                const std::vector<SubEntry>& subscriptions);

  ~RithmicEngine();

  RithmicEngine(const RithmicEngine&) = delete;
  RithmicEngine& operator=(const RithmicEngine&) = delete;

  bool Start();
  void Stop();

  RApi::REngine* engine() const noexcept { return pEngine_.get(); }
  const std::vector<SubEntry>& subscriptions() const noexcept { return subscriptions_; }

 private:
  bool BuildFakeEnvp();
  bool CreateEngineAndCallbacks();
  bool LoginRepository();
  bool LoginMarketData();
  bool SubscribeAll();

  const Config& config_;
  shm_layout::TickQueue& tick_queue_;
  shm_layout::DepthQueue& depth_queue_;
  shm_layout::BookTickerQueue& book_ticker_queue_;
  const RithmicChannelMap& channel_map_;
  SsboeConverter& converter_;
  std::vector<SubEntry> subscriptions_;

  std::vector<std::string> env_strings_;
  std::vector<char*> fake_envp_;

  std::unique_ptr<MyAdmCallbacks> pAdmCallbacks_;
  std::unique_ptr<RApi::REngine> pEngine_;
  std::unique_ptr<MyCallbacks> pCallbacks_;
};

}  // namespace rithmic
}  // namespace sqc
