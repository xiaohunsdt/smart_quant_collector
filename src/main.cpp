#include <string>

#include "common/logger_init.h"
#include "common/signal_handler.h"
#include "config/config_loader.h"
#include "exchange/channel_mapping.h"
#include "exchange/crypto/crypto_factory.h"
#include "exchange/rithmic/rithmic_process_manager.h"
#include "pubsub/pub_worker.h"
#include "quill/LogMacros.h"
#include "storage/storage_router.h"
#include "telemetry/telemetry_agent.h"

int main(int argc, char* argv[]) {
  using namespace sqc;
  std::string config_path = "config/config.yaml";
  if(argc > 1) config_path = argv[1];

  Config::Load(config_path);
  InitLogger();
  LOG_INFO(GetLogger(), "smart_quant_collector v1.0.0 starting");

  SignalHandler::Install();

  // 1. Singletons — touch them here to force construction during single-threaded init.
  ChannelRegistry::Instance();
  StorageRouter::Instance();

  // 2. Telemetry
  TelemetryAgent::Init();

  // 3-4. Crypto subsystem — validates config, owns io_ctx/ssl_ctx/channels/parser_pool
  auto crypto = BuildCryptoSubsystem();
  if(!crypto) return EXIT_FAILURE;

  // 5. Pub/Sub — reads endpoints and shard count from Config internally
  PubWorker::Init();

  // Freeze channel registrations before parser threads start.
  StorageRouter::Instance().FreezeChannels();

  // 5b. Rithmic — cross-process pipeline (shared memory MPSC queue)
  //      Managed by exchange_factory; returns nullptr if no futures exchanges configured.
  auto rithmic_mgr = CreateRithmicManager(config_path);

  // 6-7. Threads
  TelemetryAgent::Instance().Start();
  crypto->RunParsers();
  crypto->RunNetwork();

  PubWorker::Instance().Start();

  LOG_INFO(GetLogger(), "All moudle started, waiting for shutdown");

  // 8. Shutdown
  SignalHandler::WaitForShutdown();
  LOG_INFO(GetLogger(), "Shutdown signal received");

  crypto->StopNetwork();

  // Rithmic shutdown
  if(rithmic_mgr) rithmic_mgr->Shutdown();

  crypto->DrainQueues();
  crypto->Shutdown();

  StorageRouter::Instance().FlushAndClose();

  TelemetryAgent::Instance().Stop();
  PubWorker::Instance().Stop();

  LOG_INFO(GetLogger(), "smart_quant_collector shutdown complete");
  return 0;
}
