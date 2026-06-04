#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "src/config/secure_string.h"
#include "src/exchange/rithmic/rithmic_callbacks.h"
#include "src/exchange/rithmic/rithmic_queue.h"
#include "src/exchange/rithmic/rithmic_types.h"

namespace RApi { class REngine; }
namespace sqc {
struct ChannelInfo; class ChannelRegistry; class StorageRouter;

namespace rithmic {

class RithmicEngine {
 public:
  struct Config {
    bool enabled = false;
    std::string mode = "paper";
    std::string user;
    SecureString password;
    uint32_t depth_level = 10;
    uint32_t engine_core = 8;
  };

  RithmicEngine(Config config,
                std::shared_ptr<MpscRithmicQueue<>> queue,
                RithmicChannelMap channel_map,
                ChannelRegistry* channel_registry,
                StorageRouter* storage_router);
  ~RithmicEngine();

  RithmicEngine(const RithmicEngine&) = delete;
  RithmicEngine& operator=(const RithmicEngine&) = delete;

  void Start();
  void Stop();
  [[nodiscard]] bool IsLoggedIn() const noexcept;

 private:
  void RunLoop();
  void Cleanup();                    // Fix #6, #11: unified cleanup
  bool LoginRepository();
  bool CheckAgreements();
  bool LoginMarketData();
  bool SubscribeAll();
  bool WaitForLogin(std::atomic<int>& status, int timeout_sec);

  Config config_;
  std::shared_ptr<MpscRithmicQueue<>> queue_;
  RithmicChannelMap channel_map_;
  ChannelRegistry* channel_registry_;
  StorageRouter* storage_router_;

  // RApi objects — created/destroyed on engine thread
  struct RApiObjects;
  std::unique_ptr<RApiObjects> rapi_;

  // Callbacks + converter (owned by engine)
  std::unique_ptr<RithmicCallbacks> callbacks_;
  SsboeConverter converter_;

  // Fix #8: env vars stored as member (not stack-local) to avoid UAF
  std::vector<std::string> env_strings_;

  std::thread thread_;
  std::atomic<bool> running_{false};

  std::atomic<int> rep_login_status_{kLoginNotLoggedIn};
  std::atomic<int> md_login_status_{kLoginNotLoggedIn};
};

}  // namespace rithmic
}  // namespace sqc
