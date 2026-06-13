#ifndef RITHMIC_CALLBACKS_H
#define RITHMIC_CALLBACKS_H

#include "RApiPlus.h"

#include <atomic>
#include <cstdint>
#include <unordered_map>

// Note: 'using namespace RApi' is kept here because all 50+ virtual method
// signatures must exactly match the base class types.
using namespace RApi;

#include "src/exchange/rithmic/rithmic_shm.h"
#include "src/exchange/rithmic/rithmic_types.h"
#include "rithmic_orderbook.h"
#include "quill/Logger.h"
#include "quill/SimpleSetup.h"


// ============================================================================
// Global login state atomics (exactly matching rithmic_md_saver pattern)
// ============================================================================

extern std::atomic<int>  g_iRepLoginStatus;
extern std::atomic<bool> g_bRcvdUnacceptedAgreements;
extern std::atomic<int>  g_iUnacceptedMandatoryAgreements;
extern std::atomic<int>  g_iMdLoginStatus;

constexpr int LoginStatus_NotLoggedIn     = 0;
constexpr int LoginStatus_AwaitingResults = 1;
constexpr int LoginStatus_Failed          = 2;
constexpr int LoginStatus_Complete        = 3;

// ============================================================================
// MyAdmCallbacks
// ============================================================================

class MyAdmCallbacks : public AdmCallbacks {
 public:
  MyAdmCallbacks() = default;
  ~MyAdmCallbacks() override = default;
  int Alert(AlertInfo* pInfo, void* pContext, int* aiCode) override;
};

// ============================================================================
// MyCallbacks
// ============================================================================

class MyCallbacks : public RCallbacks {
 public:
  MyCallbacks(sqc::rithmic::shm_layout::TickQueue& tick_queue,
              sqc::rithmic::shm_layout::DepthQueue& depth_queue,
              sqc::rithmic::shm_layout::BookTickerQueue& book_ticker_queue,
              const sqc::rithmic::RithmicChannelMap& channel_map,
              sqc::rithmic::SsboeConverter& converter,
              uint32_t max_depth_levels);

  ~MyCallbacks() override = default;

  // Login / agreement
  int Alert(AlertInfo* pInfo, void* pContext, int* aiCode) override;
  int AgreementList(AgreementListInfo* pInfo, void* pContext, int* aiCode) override;

  // Hot-path market data
  int TradePrint(TradeInfo* pInfo, void* pContext, int* aiCode) override;
  int BestBidAskQuote(BidInfo* pBid, AskInfo* pAsk, void* pContext, int* aiCode) override;
  int LimitOrderBook(LimitOrderBookInfo* pInfo, void* pContext, int* aiCode) override;
  int TradeCondition(TradeInfo* pInfo, void* pContext, int* aiCode) override;

  // All other callbacks: stub with RECV_OK
  int AskQuote(AskInfo*, void*, int* aiCode) override;
  int BestAskQuote(AskInfo*, void*, int* aiCode) override;
  int BestBidQuote(BidInfo*, void*, int* aiCode) override;
  int BidQuote(BidInfo*, void*, int* aiCode) override;
  int BinaryContractList(BinaryContractListInfo*, void*, int* aiCode) override;
  int ClosePrice(ClosePriceInfo*, void*, int* aiCode) override;
  int ClosingIndicator(ClosingIndicatorInfo*, void*, int* aiCode) override;
  int EndQuote(EndQuoteInfo*, void*, int* aiCode) override;
  int EquityOptionStrategyList(EquityOptionStrategyListInfo*, void*, int* aiCode) override;
  int HighPrice(HighPriceInfo*, void*, int* aiCode) override;
  int InstrumentByUnderlying(InstrumentByUnderlyingInfo*, void*, int* aiCode) override;
  int InstrumentSearch(InstrumentSearchInfo*, void*, int* aiCode) override;
  int LowPrice(LowPriceInfo*, void*, int* aiCode) override;
  int MarketMode(MarketModeInfo*, void*, int* aiCode) override;
  int MidPrice(MidPriceInfo*, void*, int* aiCode) override;
  int OpenInterest(OpenInterestInfo*, void*, int* aiCode) override;
  int OpenPrice(OpenPriceInfo*, void*, int* aiCode) override;
  int OpeningIndicator(OpeningIndicatorInfo*, void*, int* aiCode) override;
  int OptionList(OptionListInfo*, void*, int* aiCode) override;
  int RefData(RefDataInfo*, void*, int* aiCode) override;
  int SettlementPrice(SettlementPriceInfo*, void*, int* aiCode) override;
  int Strategy(StrategyInfo*, void*, int* aiCode) override;
  int StrategyList(StrategyListInfo*, void*, int* aiCode) override;
  int TradeReplay(TradeReplayInfo*, void*, int* aiCode) override;
  int TradeRoute(TradeRouteInfo*, void*, int* aiCode) override;
  int TradeRouteList(TradeRouteListInfo*, void*, int* aiCode) override;
  int TradeVolume(TradeVolumeInfo*, void*, int* aiCode) override;
  int Bar(BarInfo*, void*, int* aiCode) override;
  int BarReplay(BarReplayInfo*, void*, int* aiCode) override;
  int AccountList(AccountListInfo*, void*, int* aiCode) override;
  int PasswordChange(PasswordChangeInfo*, void*, int* aiCode) override;
  int ExchangeList(ExchangeListInfo*, void*, int* aiCode) override;
  int ExecutionReplay(ExecutionReplayInfo*, void*, int* aiCode) override;
  int LineUpdate(LineInfo*, void*, int* aiCode) override;
  int OpenOrderReplay(OrderReplayInfo*, void*, int* aiCode) override;
  int OrderReplay(OrderReplayInfo*, void*, int* aiCode) override;
  int PnlReplay(PnlReplayInfo*, void*, int* aiCode) override;
  int PnlUpdate(PnlInfo*, void*, int* aiCode) override;
  int PriceIncrUpdate(PriceIncrInfo*, void*, int* aiCode) override;
  int ProductRmsList(ProductRmsListInfo*, void*, int* aiCode) override;
  int SingleOrderReplay(SingleOrderReplayInfo*, void*, int* aiCode) override;
  int BustReport(OrderBustReport*, void*, int* aiCode) override;
  int CancelReport(OrderCancelReport*, void*, int* aiCode) override;
  int FailureReport(OrderFailureReport*, void*, int* aiCode) override;
  int FillReport(OrderFillReport*, void*, int* aiCode) override;
  int ModifyReport(OrderModifyReport*, void*, int* aiCode) override;
  int NotCancelledReport(OrderNotCancelledReport*, void*, int* aiCode) override;
  int NotModifiedReport(OrderNotModifiedReport*, void*, int* aiCode) override;
  int RejectReport(OrderRejectReport*, void*, int* aiCode) override;
  int StatusReport(OrderStatusReport*, void*, int* aiCode) override;
  int TradeCorrectReport(OrderTradeCorrectReport*, void*, int* aiCode) override;
  int TriggerPulledReport(OrderTriggerPulledReport*, void*, int* aiCode) override;
  int TriggerReport(OrderTriggerReport*, void*, int* aiCode) override;
  int OtherReport(OrderReport*, void*, int* aiCode) override;
  int SodUpdate(SodReport*, void*, int* aiCode) override;
  int Quote(QuoteReport*, void*, int* aiCode) override;

 private:
  quill::Logger* logger = quill::simple_logger("stdout");

  sqc::rithmic::shm_layout::TickQueue& m_tickQueue;
  sqc::rithmic::shm_layout::DepthQueue& m_depthQueue;
  sqc::rithmic::shm_layout::BookTickerQueue& m_bookTickerQueue;
  const sqc::rithmic::RithmicChannelMap& m_channelMap;
  sqc::rithmic::SsboeConverter& m_converter;
  uint32_t m_maxDepthLevels;

  // ---- Per-symbol order books ----
  static constexpr uint32_t kMaxBooks = 8;
  static constexpr uint8_t kInvalidBookIdx = 0xFF;

  sqc::rithmic::RithmicOrderBook m_books[kMaxBooks];
  std::unordered_map<uint32_t, uint8_t> m_channelToBook;  // channel_id → m_books index
  uint32_t m_bookCount = 0;

  sqc::rithmic::RithmicOrderBook* FindBook(std::string_view exchange,
                                           std::string_view ticker);
  void PushSnapshot(sqc::rithmic::RithmicOrderBook& book, uint64_t exchange_ts);
};

#endif  // RITHMIC_CALLBACKS_H
