#include "callbacks.h"  // must come before engine.h for MyAdmCallbacks/MyCallbacks
#include "rithmic_engine.h"

#include <cstring>
#include <unistd.h>

#include "quill/Frontend.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// Construction / Destruction
// ============================================================================

RithmicEngine::RithmicEngine(const Config& config,
                               shm_layout::TickQueue& tick_queue,
                               shm_layout::DepthQueue& depth_queue,
                               shm_layout::BookTickerQueue& book_ticker_queue,
                               const RithmicChannelMap& channel_map,
                               SsboeConverter& converter,
                               const std::vector<SubEntry>& subscriptions)
    : config_(config),
      tick_queue_(tick_queue),
      depth_queue_(depth_queue),
      book_ticker_queue_(book_ticker_queue),
      channel_map_(channel_map),
      converter_(converter),
      subscriptions_(subscriptions) {}

RithmicEngine::~RithmicEngine() = default;

// ============================================================================
// BuildFakeEnvp (matching rithmic_md_saver pattern)
// ============================================================================

bool RithmicEngine::BuildFakeEnvp() {
  env_strings_.clear();
  fake_envp_.clear();

  env_strings_.push_back("MML_SSL_CLNT_AUTH_FILE=" + config_.ssl_cert_file);

  if (!config_.domain_servers.empty()) {
      env_strings_.push_back("MML_DMN_SRVR_ADDR=" + config_.domain_servers);
  }

  env_strings_.push_back("MML_DOMAIN_NAME=" + config_.domain_name);

  if (!config_.license_servers.empty()) {
      env_strings_.push_back("MML_LIC_SRVR_ADDR=" + config_.license_servers);
  }

  if (!config_.local_broker.empty()) {
      env_strings_.push_back("MML_LOC_BROK_ADDR=" + config_.local_broker);
  }

  if (!config_.logger_servers.empty()) {
      env_strings_.push_back("MML_LOGGER_ADDR=" + config_.logger_servers);
  }

  env_strings_.push_back("MML_LOG_TYPE=log_net");
  env_strings_.push_back("USER=" + config_.user);

  for (auto& s : env_strings_) {
      fake_envp_.push_back(&s[0]);
  }
  fake_envp_.push_back(nullptr);

  return true;
}

// ============================================================================
// CreateEngineAndCallbacks
// ============================================================================

bool RithmicEngine::CreateEngineAndCallbacks() {
  if (!BuildFakeEnvp()) return false;

  REngineParams oParams;
  oParams.sAppName.pData        = const_cast<char*>(config_.app_name.c_str());
  oParams.sAppName.iDataLen     = static_cast<int>(config_.app_name.length());
  oParams.sAppVersion.pData     = const_cast<char*>(config_.app_version.c_str());
  oParams.sAppVersion.iDataLen  = static_cast<int>(config_.app_version.length());
  oParams.envp                  = fake_envp_.data();
  oParams.sLogFilePath.pData    = const_cast<char*>("rithmic_gateway.log");
  oParams.sLogFilePath.iDataLen = 20;

  try {
      pAdmCallbacks_ = std::make_unique<MyAdmCallbacks>();
  } catch (OmneException& oEx) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "MyAdmCallbacks error: {}", oEx.getErrorCode());
      return false;
  }
  oParams.pAdmCallbacks = pAdmCallbacks_.get();

  LOG_INFO(quill::Frontend::get_logger("stdout"), "Creating REngine...");
  try {
      pEngine_ = std::make_unique<REngine>(&oParams);
  } catch (OmneException& oEx) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "REngine error: {}", oEx.getErrorCode());
      return false;
  }
  LOG_INFO(quill::Frontend::get_logger("stdout"), "REngine created.");

  try {
    //   pCallbacks_ = std::make_unique<MyCallbacks>(tick_queue_, depth_queue_, book_ticker_queue_,
    //                                                     channel_map_, converter_, sqc::kMaxOrderbookLevels);
    pCallbacks_ = std::make_unique<MyCallbacks>();
  } catch (OmneException& oEx) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "MyCallbacks error: {}", oEx.getErrorCode());
      return false;
  }

  return true;
}

// ============================================================================
// LoginRepository (matching rithmic_md_saver)
// ============================================================================

bool RithmicEngine::LoginRepository() {
  tsNCharcb sRepEnvKey, sRepUser, sRepPassword, sRepCnnctPt;

  sRepEnvKey.pData    = const_cast<char*>("system");
  sRepEnvKey.iDataLen = (int)strlen(sRepEnvKey.pData);
  sRepUser.pData      = const_cast<char*>(config_.user.c_str());
  sRepUser.iDataLen   = (int)config_.user.length();
  sRepPassword.pData  = const_cast<char*>(config_.password.get().c_str());
  sRepPassword.iDataLen = (int)config_.password.get().length();
  sRepCnnctPt.pData   = const_cast<char*>(config_.repository_connect_pt.c_str());
  sRepCnnctPt.iDataLen = (int)config_.repository_connect_pt.length();

  int iCode;
  if (!pEngine_->loginRepository(&sRepEnvKey, &sRepUser, &sRepPassword, &sRepCnnctPt, pCallbacks_.get(), &iCode)) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "loginRepository error: {}", iCode);
      return false;
  }

  while (g_iRepLoginStatus != LoginStatus_Complete && g_iRepLoginStatus != LoginStatus_Failed) {
      sleep(1);
  }

  if (g_iRepLoginStatus == LoginStatus_Failed) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "Repository login failed.");
      return false;
  }

  if (!pEngine_->listAgreements(false, nullptr, &iCode)) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "listAgreements error: {}", iCode);
      pEngine_->logoutRepository(&iCode);
      return false;
  }

  while (!g_bRcvdUnacceptedAgreements) {
      sleep(1);
  }

  if (g_iUnacceptedMandatoryAgreements > 0) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"),
                "Unaccepted mandatory agreements: {}. Please log in via R|Trader and accept them.",
                g_iUnacceptedMandatoryAgreements.load());
      pEngine_->logoutRepository(&iCode);
      return false;
  }

  if (!pEngine_->logoutRepository(&iCode)) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "logoutRepository error: {}", iCode);
      return false;
  }

  return true;
}

// ============================================================================
// LoginMarketData (matching rithmic_md_saver)
// ============================================================================

bool RithmicEngine::LoginMarketData() {
  LoginParams oLoginParams;
  oLoginParams.pCallbacks = pCallbacks_.get();

  oLoginParams.sMdUser.pData      = const_cast<char*>(config_.user.c_str());
  oLoginParams.sMdUser.iDataLen   = (int)config_.user.length();
  oLoginParams.sMdPassword.pData  = const_cast<char*>(config_.password.get().c_str());
  oLoginParams.sMdPassword.iDataLen = (int)config_.password.get().length();
  const std::string md_cnnct_pt =
      (config_.use_aggregated_md && !config_.md_connect_pt_agg.empty())
          ? config_.md_connect_pt_agg : config_.md_connect_pt;
  oLoginParams.sMdCnnctPt.pData   = const_cast<char*>(md_cnnct_pt.c_str());
  oLoginParams.sMdCnnctPt.iDataLen = (int)md_cnnct_pt.length();

  oLoginParams.sTsUser.pData      = const_cast<char*>(config_.user.c_str());
  oLoginParams.sTsUser.iDataLen   = (int)config_.user.length();
  oLoginParams.sTsPassword.pData  = const_cast<char*>(config_.password.get().c_str());
  oLoginParams.sTsPassword.iDataLen = (int)config_.password.get().length();
  oLoginParams.sTsCnnctPt.pData   = const_cast<char*>(config_.ts_connect_pt.c_str());
  oLoginParams.sTsCnnctPt.iDataLen = (int)config_.ts_connect_pt.length();

  // Optional history (IH) login
  if (!config_.ih_connect_pt.empty()) {
      oLoginParams.sIhUser.pData = const_cast<char*>(config_.user.c_str());
      oLoginParams.sIhUser.iDataLen = (int)config_.user.length();
      oLoginParams.sIhPassword.pData = const_cast<char*>(config_.password.get().c_str());
      oLoginParams.sIhPassword.iDataLen = (int)config_.password.get().length();
      oLoginParams.sIhCnnctPt.pData = const_cast<char*>(config_.ih_connect_pt.c_str());
      oLoginParams.sIhCnnctPt.iDataLen = (int)config_.ih_connect_pt.length();
  }

  // Optional PnL connect point (no separate user/pass fields in LoginParams)
  if (!config_.pnl_connect_pt.empty()) {
      oLoginParams.sPnlCnnctPt.pData = const_cast<char*>(config_.pnl_connect_pt.c_str());
      oLoginParams.sPnlCnnctPt.iDataLen = (int)config_.pnl_connect_pt.length();
  }

  int iCode;
  if (!pEngine_->login(&oLoginParams, &iCode)) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "login error: {}", iCode);
      return false;
  }

  while (g_iMdLoginStatus != LoginStatus_Complete && g_iMdLoginStatus != LoginStatus_Failed) {
      sleep(1);
  }

  if (g_iMdLoginStatus == LoginStatus_Failed) {
      LOG_ERROR(quill::Frontend::get_logger("stdout"), "Market data login failed.");
      return false;
  }

  LOG_INFO(quill::Frontend::get_logger("stdout"), "Market data login complete.");
  return true;
}

// ============================================================================
// SubscribeAll (matching rithmic_md_saver: MD_ALL)
// ============================================================================

bool RithmicEngine::SubscribeAll() {
  int iFlags = MD_ALL;
  int iCode;

  for (const auto& sub : subscriptions_) {
    if (!sub.enabled) continue;
      tsNCharcb sExchange, sTicker;

      sExchange.pData    = const_cast<char*>(sub.exchange.c_str());
      sExchange.iDataLen = sub.exchange.length();
      sTicker.pData      = const_cast<char*>(sub.ticker.c_str());
      sTicker.iDataLen   = sub.ticker.length();

      LOG_INFO(quill::Frontend::get_logger("stdout"), "Subscribing: {}/{}", sub.exchange, sub.ticker);

      if (!pEngine_->subscribe(&sExchange, &sTicker, iFlags, &iCode)) {
          LOG_ERROR(quill::Frontend::get_logger("stdout"), "subscribe error for {}/{}: {}", sub.exchange, sub.ticker, iCode);
      }
  }

  return true;
}

// ============================================================================
// Start / Stop
// ============================================================================

bool RithmicEngine::Start() {
  if (!CreateEngineAndCallbacks()) return false;
  if (!LoginRepository()) return false;
  if (!LoginMarketData()) return false;
  if (!SubscribeAll()) return false;
  return true;
}

void RithmicEngine::Stop() {
  if (!pEngine_) return;

  int iCode;
  for (const auto& sub : subscriptions_) {
      tsNCharcb sExchange, sTicker;
      sExchange.pData    = const_cast<char*>(sub.exchange.c_str());
      sExchange.iDataLen = sub.exchange.length();
      sTicker.pData      = const_cast<char*>(sub.ticker.c_str());
      sTicker.iDataLen   = sub.ticker.length();
      pEngine_->unsubscribe(&sExchange, &sTicker, &iCode);
  }

  pEngine_->logout(&iCode);
  sleep(1);
}

}  // namespace rithmic
}  // namespace sqc
