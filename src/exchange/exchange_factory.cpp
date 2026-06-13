#include <memory>

#include "config/config_loader.h"
#include "exchange/exchange_adapter.h"
#include "exchange/rithmic/rithmic_process_manager.h"

namespace sqc {

std::unique_ptr<rithmic::RithmicProcessManager> CreateRithmicManager(
    std::string_view config_path) {
  const auto& rcfg = Config::Instance().rithmic;

  rithmic::RithmicProcessManager::Config proc_cfg;
  proc_cfg.config_path = config_path;
  proc_cfg.engine_core = rcfg.engine_core;
  proc_cfg.forwarder_core = rcfg.forwarder_core;
  proc_cfg.cpu_affinity = Config::Instance().global.cpu_affinity;

  auto mgr = std::make_unique<rithmic::RithmicProcessManager>(proc_cfg);
  if (!mgr->Setup()) return nullptr;
  return mgr;
}

}  // namespace sqc
