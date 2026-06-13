#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "RApiPlus.h"

#include <atomic>

// Note: 'using namespace RApi' is kept here because all 50+ virtual method
// signatures must exactly match the base class types. Qualifying each one
// would add RApi:: to every parameter with no readability benefit.
// This header is only included by callbacks.cpp and main.cpp — not a public API.
using namespace RApi;

class DbWriter;

extern std::atomic<int>  g_iRepLoginStatus;
extern std::atomic<bool> g_bRcvdUnacceptedAgreements;
extern std::atomic<int>  g_iUnacceptedMandatoryAgreements;
extern std::atomic<int>  g_iMdLoginStatus;

constexpr int LoginStatus_NotLoggedIn     = 0;
constexpr int LoginStatus_AwaitingResults = 1;
constexpr int LoginStatus_Failed          = 2;
constexpr int LoginStatus_Complete        = 3;

class MyAdmCallbacks : public AdmCallbacks {
public:
    MyAdmCallbacks() = default;
    ~MyAdmCallbacks() override = default;
    int Alert(AlertInfo* pInfo, void* pContext, int* aiCode) override;
};

class MyCallbacks : public RCallbacks {
public:
    MyCallbacks() = default;
    ~MyCallbacks() override = default;

    int Alert(AlertInfo* pInfo, void* pContext, int* aiCode) override;
    int AgreementList(AgreementListInfo* pInfo, void* pContext, int* aiCode) override;

    int AskQuote(AskInfo* pInfo, void* pContext, int* aiCode) override;
    int BestAskQuote(AskInfo* pInfo, void* pContext, int* aiCode) override;
    int BestBidAskQuote(BidInfo* pBid, AskInfo* pAsk, void* pContext, int* aiCode) override;
    int BestBidQuote(BidInfo* pInfo, void* pContext, int* aiCode) override;
    int BidQuote(BidInfo* pInfo, void* pContext, int* aiCode) override;
    int BinaryContractList(BinaryContractListInfo* pInfo, void* pContext, int* aiCode) override;
    int ClosePrice(ClosePriceInfo* pInfo, void* pContext, int* aiCode) override;
    int ClosingIndicator(ClosingIndicatorInfo* pInfo, void* pContext, int* aiCode) override;
    int EndQuote(EndQuoteInfo* pInfo, void* pContext, int* aiCode) override;
    int EquityOptionStrategyList(EquityOptionStrategyListInfo* pInfo, void* pContext, int* aiCode) override;
    int HighPrice(HighPriceInfo* pInfo, void* pContext, int* aiCode) override;
    int InstrumentByUnderlying(InstrumentByUnderlyingInfo* pInfo, void* pContext, int* aiCode) override;
    int InstrumentSearch(InstrumentSearchInfo* pInfo, void* pContext, int* aiCode) override;
    int LimitOrderBook(LimitOrderBookInfo* pInfo, void* pContext, int* aiCode) override;
    int LowPrice(LowPriceInfo* pInfo, void* pContext, int* aiCode) override;
    int MarketMode(MarketModeInfo* pInfo, void* pContext, int* aiCode) override;
    int MidPrice(MidPriceInfo* pInfo, void* pContext, int* aiCode) override;
    int OpenInterest(OpenInterestInfo* pInfo, void* pContext, int* aiCode) override;
    int OpenPrice(OpenPriceInfo* pInfo, void* pContext, int* aiCode) override;
    int OpeningIndicator(OpeningIndicatorInfo* pInfo, void* pContext, int* aiCode) override;
    int OptionList(OptionListInfo* pInfo, void* pContext, int* aiCode) override;
    int RefData(RefDataInfo* pInfo, void* pContext, int* aiCode) override;
    int SettlementPrice(SettlementPriceInfo* pInfo, void* pContext, int* aiCode) override;
    int Strategy(StrategyInfo* pInfo, void* pContext, int* aiCode) override;
    int StrategyList(StrategyListInfo* pInfo, void* pContext, int* aiCode) override;
    int TradeCondition(TradeInfo* pInfo, void* pContext, int* aiCode) override;
    int TradePrint(TradeInfo* pInfo, void* pContext, int* aiCode) override;
    int TradeReplay(TradeReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int TradeRoute(TradeRouteInfo* pInfo, void* pContext, int* aiCode) override;
    int TradeRouteList(TradeRouteListInfo* pInfo, void* pContext, int* aiCode) override;
    int TradeVolume(TradeVolumeInfo* pInfo, void* pContext, int* aiCode) override;
    int Bar(BarInfo* pInfo, void* pContext, int* aiCode) override;
    int BarReplay(BarReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int AccountList(AccountListInfo* pInfo, void* pContext, int* aiCode) override;
    int PasswordChange(PasswordChangeInfo* pInfo, void* pContext, int* aiCode) override;
    int ExchangeList(ExchangeListInfo* pInfo, void* pContext, int* aiCode) override;
    int ExecutionReplay(ExecutionReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int LineUpdate(LineInfo* pInfo, void* pContext, int* aiCode) override;
    int OpenOrderReplay(OrderReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int OrderReplay(OrderReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int PnlReplay(PnlReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int PnlUpdate(PnlInfo* pInfo, void* pContext, int* aiCode) override;
    int PriceIncrUpdate(PriceIncrInfo* pInfo, void* pContext, int* aiCode) override;
    int ProductRmsList(ProductRmsListInfo* pInfo, void* pContext, int* aiCode) override;
    int SingleOrderReplay(SingleOrderReplayInfo* pInfo, void* pContext, int* aiCode) override;
    int BustReport(OrderBustReport* pReport, void* pContext, int* aiCode) override;
    int CancelReport(OrderCancelReport* pReport, void* pContext, int* aiCode) override;
    int FailureReport(OrderFailureReport* pReport, void* pContext, int* aiCode) override;
    int FillReport(OrderFillReport* pReport, void* pContext, int* aiCode) override;
    int ModifyReport(OrderModifyReport* pReport, void* pContext, int* aiCode) override;
    int NotCancelledReport(OrderNotCancelledReport* pReport, void* pContext, int* aiCode) override;
    int NotModifiedReport(OrderNotModifiedReport* pReport, void* pContext, int* aiCode) override;
    int RejectReport(OrderRejectReport* pReport, void* pContext, int* aiCode) override;
    int StatusReport(OrderStatusReport* pReport, void* pContext, int* aiCode) override;
    int TradeCorrectReport(OrderTradeCorrectReport* pReport, void* pContext, int* aiCode) override;
    int TriggerPulledReport(OrderTriggerPulledReport* pReport, void* pContext, int* aiCode) override;
    int TriggerReport(OrderTriggerReport* pReport, void* pContext, int* aiCode) override;
    int OtherReport(OrderReport* pReport, void* pContext, int* aiCode) override;
    int SodUpdate(SodReport* pReport, void* pContext, int* aiCode) override;
    int Quote(QuoteReport* pReport, void* pContext, int* aiCode) override;

};

#endif
