#include "src/exchange/rithmic/rithmic_engine.h"

#include <chrono>
#include <cstring>
#include <thread>

#include "RApiPlus.h"
#include "quill/LogMacros.h"
#include "src/common/logger_init.h"
#include "src/exchange/channel_mapping.h"
#include "src/storage/storage_router.h"

namespace sqc {
namespace rithmic {

namespace {

void FillNCharcb(tsNCharcb* out, const std::string& s) {
  if (s.empty()) {
    out->pData = nullptr;
    out->iDataLen = 0;
  } else {
    out->pData = const_cast<char*>(s.data());
    out->iDataLen = static_cast<int>(s.size());
  }
}

}  // namespace

// Fix #6: unique_ptr with custom deleter replaces raw pointer
struct RithmicEngine::RApiObjects {
  std::unique_ptr<RApi::REngine, void(*)(RApi::REngine*)> engine{
      nullptr, [](RApi::REngine* e) { delete e; }};
};

RithmicEngine::RithmicEngine(Config config,
                               std::shared_ptr<MpscRithmicQueue<>> queue,
                               RithmicChannelMap channel_map,
                               ChannelRegistry* channel_registry,
                               StorageRouter* storage_router)
    : config_(std::move(config)),
      queue_(std::move(queue)),
      channel_map_(std::move(channel_map)),
      channel_registry_(channel_registry),
      storage_router_(storage_router),
      rapi_(std::make_unique<RApiObjects>()) {}

RithmicEngine::~RithmicEngine() { Stop(); }

void RithmicEngine::Start() {
  running_.store(true, std::memory_order_release);
  thread_ = std::thread(&RithmicEngine::RunLoop, this);
}

void RithmicEngine::Stop() {
  running_.store(false, std::memory_order_release);
  if (thread_.joinable()) thread_.join();
}

bool RithmicEngine::IsLoggedIn() const noexcept {
  return md_login_status_.load(std::memory_order_acquire) == kLoginComplete;
}

// Fix #6, #11: unified cleanup for all code paths
void RithmicEngine::Cleanup() {
  if (rapi_->engine) {
    int code;
    rapi_->engine->logout(&code);
    rapi_->engine.reset();
  }
  callbacks_.reset();
}

void RithmicEngine::RunLoop() {
  LOG_INFO(GetLogger(), "RithmicEngine: starting login (mode={})", config_.mode);

  if (!LoginRepository()) {
    LOG_ERROR(GetLogger(), "RithmicEngine: repository login failed");
    Cleanup();
    return;
  }
  if (!CheckAgreements()) {
    LOG_WARNING(GetLogger(), "RithmicEngine: agreement check had issues");
  }
  if (!LoginMarketData()) {
    LOG_ERROR(GetLogger(), "RithmicEngine: market data login failed");
    Cleanup();
    return;
  }

  LOG_INFO(GetLogger(), "RithmicEngine: login complete, {} symbols",
           channel_map_.Size());

  if (!SubscribeAll()) {
    LOG_ERROR(GetLogger(), "RithmicEngine: subscription failed");
    Cleanup();
    return;
  }

  while (running_.load(std::memory_order_acquire)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  LOG_INFO(GetLogger(), "RithmicEngine: shutting down");
  Cleanup();
}

bool RithmicEngine::WaitForLogin(std::atomic<int>& status, int timeout_sec) {
  for (int i = 0; i < timeout_sec * 10; ++i) {
    int s = status.load(std::memory_order_acquire);
    if (s == kLoginComplete) return true;
    if (s == kLoginFailed) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  LOG_ERROR(GetLogger(), "RithmicEngine: login timed out after {}s", timeout_sec);
  return false;
}

bool RithmicEngine::LoginRepository() {
  // Fix #8: env strings stored as member vector (not stack-local)
  env_strings_.clear();
  if (config_.mode == "paper") {
    env_strings_.push_back("MML_DMN_SRVR_ADDR=rituz00100.00.rithmic.com:65000~"
                           "rituz00100.00.rithmic.net:65000~rituz00100.00.rithmic.com:65000~"
                           "rituz00100.00.rithmic.net:65000");
    env_strings_.push_back("MML_DOMAIN_NAME=rithmic_uat_dmz_domain");
    env_strings_.push_back("MML_LIC_SRVR_ADDR=rituz00100.00.rithmic.com:56000~"
                           "rituz00100.00.rithmic.net:56000~rituz00100.00.rithmic.com:56000~"
                           "rituz00100.00.rithmic.net:56000");
    env_strings_.push_back("MML_LOC_BROK_ADDR=rituz00100.00.rithmic.com:64100");
    env_strings_.push_back("MML_LOGGER_ADDR=rituz00100.00.rithmic.com:45454~"
                           "rituz00100.00.rithmic.net:45454~rituz00100.00.rithmic.com:45454~"
                           "rituz00100.00.rithmic.net:45454");
    env_strings_.push_back("MML_LOG_TYPE=log_net");
    env_strings_.push_back("MML_SSL_CLNT_AUTH_FILE=etc/rithmic_ssl_cert_auth_params");
  } else {
    env_strings_.push_back("MML_DMN_SRVR_ADDR=rituz00100.00.rithmic.com:65000~"
                           "rituz00100.00.rithmic.net:65000~rituz00100.00.rithmic.com:65000~"
                           "rituz00100.00.rithmic.net:65000");
    env_strings_.push_back("MML_DOMAIN_NAME=rithmic_prod_dmz_domain");
    env_strings_.push_back("MML_LIC_SRVR_ADDR=rituz00100.00.rithmic.com:56000~"
                           "rituz00100.00.rithmic.net:56000~rituz00100.00.rithmic.com:56000~"
                           "rituz00100.00.rithmic.net:56000");
    env_strings_.push_back("MML_LOC_BROK_ADDR=rituz00100.00.rithmic.com:64100");
    env_strings_.push_back("MML_LOGGER_ADDR=rituz00100.00.rithmic.com:45454~"
                           "rituz00100.00.rithmic.net:45454~rituz00100.00.rithmic.com:45454~"
                           "rituz00100.00.rithmic.net:45454");
    env_strings_.push_back("MML_LOG_TYPE=log_net");
    env_strings_.push_back("MML_SSL_CLNT_AUTH_FILE=etc/rithmic_ssl_cert_auth_params");
  }

  // Build null-terminated envp array from member strings
  std::vector<const char*> envp;
  envp.reserve(env_strings_.size() + 1);
  for (const auto& s : env_strings_) envp.push_back(s.c_str());
  envp.push_back(nullptr);

  callbacks_ = std::make_unique<RithmicCallbacks>(
      *queue_, channel_map_, converter_,
      rep_login_status_, md_login_status_,
      config_.depth_level);

  RApi::REngineParams params;
  params.sAppName.pData = const_cast<char*>("SmartQuantCollector");
  params.sAppName.iDataLen = static_cast<int>(std::strlen("SmartQuantCollector"));
  params.sAppVersion.pData = const_cast<char*>("1.0.0");
  params.sAppVersion.iDataLen = static_cast<int>(std::strlen("1.0.0"));
  params.envp = (const char**)envp.data();
  params.pAdmCallbacks = callbacks_->AsAdmCallbacks();
  params.sLogFilePath.pData = const_cast<char*>("log/rithmic.log");
  params.sLogFilePath.iDataLen = static_cast<int>(std::strlen("log/rithmic.log"));

  rapi_->engine.reset(new RApi::REngine(&params));

  // Fix #2: fill credentials from config
  tsNCharcb env_key_nc{}, user_nc{}, pwd_nc{}, cnnct_pt_nc{};
  FillNCharcb(&user_nc, config_.user);
  FillNCharcb(&pwd_nc, config_.password.get());
  int code;
  rapi_->engine->loginRepository(&env_key_nc, &user_nc, &pwd_nc, &cnnct_pt_nc,
                                  callbacks_->AsRCallbacks(), &code);
  if (code != API_OK) {
    LOG_ERROR(GetLogger(), "RithmicEngine: loginRepository failed code={}", code);
    return false;
  }
  return WaitForLogin(rep_login_status_, 30);
}

bool RithmicEngine::CheckAgreements() {
  int code;
  rapi_->engine->listAgreements(false, nullptr, &code);
  return true;
}

bool RithmicEngine::LoginMarketData() {
  int code;
  rapi_->engine->logoutRepository(&code);

  RApi::LoginParams login_params;
  login_params.pCallbacks = callbacks_->AsRCallbacks();

  // Fix #2: credentials for market data login
  tsNCharcb user_nc{}, pwd_nc{};
  FillNCharcb(&user_nc, config_.user);
  FillNCharcb(&pwd_nc, config_.password.get());
  login_params.sMdUser = user_nc;
  login_params.sMdPassword = pwd_nc;

  rapi_->engine->login(&login_params, &code);
  if (code != API_OK) {
    LOG_ERROR(GetLogger(), "RithmicEngine: login failed code={}", code);
    return false;
  }
  return WaitForLogin(md_login_status_, 30);
}

// Fix #4: SubscribeAll iterates Subscriptions() list
bool RithmicEngine::SubscribeAll() {
  const auto& subs = channel_map_.Subscriptions();
  if (subs.empty()) {
    LOG_WARNING(GetLogger(), "RithmicEngine: no symbols to subscribe");
    return true;
  }

  for (const auto& sub : subs) {
    tsNCharcb exchange_nc{}, ticker_nc{};
    FillNCharcb(&exchange_nc, sub.exchange);
    FillNCharcb(&ticker_nc, sub.ticker);

    int flags = RApi::MD_PRINTS | RApi::MD_BEST | RApi::MD_QUOTES;
    int code;
    rapi_->engine->subscribe(&exchange_nc, &ticker_nc, flags, &code);
    if (code != API_OK) {
      LOG_ERROR(GetLogger(), "RithmicEngine: subscribe {}.{} failed code={}",
                sub.exchange, sub.ticker, code);
    } else {
      LOG_INFO(GetLogger(), "RithmicEngine: subscribed {}.{} flags=0x{:x}",
               sub.exchange, sub.ticker, flags);
    }
  }

  return true;
}

}  // namespace rithmic
}  // namespace sqc
