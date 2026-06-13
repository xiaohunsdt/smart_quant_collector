#include "callbacks.h"

#include <iostream>
#include <cstring>

std::atomic<int>  g_iRepLoginStatus{LoginStatus_NotLoggedIn};
std::atomic<bool> g_bRcvdUnacceptedAgreements{false};
std::atomic<int>  g_iUnacceptedMandatoryAgreements{0};
std::atomic<int>  g_iMdLoginStatus{LoginStatus_NotLoggedIn};

#define RECV_OK \
    *aiCode = API_OK; \
    return (OK);

int MyAdmCallbacks::Alert(AlertInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::Alert(AlertInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);

    if (pInfo->iConnectionId == REPOSITORY_CONNECTION_ID) {
        if (pInfo->iAlertType == ALERT_LOGIN_COMPLETE)
            g_iRepLoginStatus = LoginStatus_Complete;
        else if (pInfo->iAlertType == ALERT_LOGIN_FAILED ||
                 pInfo->iAlertType == ALERT_CONNECTION_BROKEN)
            g_iRepLoginStatus = LoginStatus_Failed;
    }

    if (pInfo->iConnectionId == MARKET_DATA_CONNECTION_ID) {
        if (pInfo->iAlertType == ALERT_LOGIN_COMPLETE)
            g_iMdLoginStatus = LoginStatus_Complete;
        else if (pInfo->iAlertType == ALERT_LOGIN_FAILED ||
                 pInfo->iAlertType == ALERT_CONNECTION_BROKEN)
            g_iMdLoginStatus = LoginStatus_Failed;
    }

    RECV_OK
}

int MyCallbacks::AgreementList(AgreementListInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);

    if (!pInfo->bAccepted) {
        tsNCharcb sActive = { const_cast<char*>("active"), 6 };
        for (int i = 0; i < pInfo->iArrayLen; i++) {
            AgreementInfo& oAg = pInfo->asAgreementInfoArray[i];
            if (oAg.sStatus.iDataLen == sActive.iDataLen &&
                memcmp(oAg.sStatus.pData, sActive.pData, oAg.sStatus.iDataLen) == 0) {
                if (oAg.bMandatory) {
                    g_iUnacceptedMandatoryAgreements++;
                }
            }
        }
        g_bRcvdUnacceptedAgreements = true;
    }

    RECV_OK
}

int MyCallbacks::AskQuote(AskInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::BestAskQuote(AskInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::BestBidAskQuote(BidInfo* pBid, AskInfo* pAsk, void* pContext, int* aiCode) {
    int iIgnored;
    pBid->dump(&iIgnored);
    pAsk->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::BestBidQuote(BidInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::BidQuote(BidInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::ClosePrice(ClosePriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::ClosingIndicator(ClosingIndicatorInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::EndQuote(EndQuoteInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::HighPrice(HighPriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::LimitOrderBook(LimitOrderBookInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::LowPrice(LowPriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::MarketMode(MarketModeInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::MidPrice(MidPriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::OpenInterest(OpenInterestInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::OpenPrice(OpenPriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::OpeningIndicator(OpeningIndicatorInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::RefData(RefDataInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::SettlementPrice(SettlementPriceInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::TradeCondition(TradeInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::TradePrint(TradeInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::TradeVolume(TradeVolumeInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

int MyCallbacks::Bar(BarInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;
    pInfo->dump(&iIgnored);
    RECV_OK
}

/* ---------- non-market-data callbacks: return OK ---------- */
int MyCallbacks::BinaryContractList(BinaryContractListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::EquityOptionStrategyList(EquityOptionStrategyListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::InstrumentByUnderlying(InstrumentByUnderlyingInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::InstrumentSearch(InstrumentSearchInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OptionList(OptionListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::Strategy(StrategyInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::StrategyList(StrategyListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeReplay(TradeReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeRoute(TradeRouteInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeRouteList(TradeRouteListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::BarReplay(BarReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::AccountList(AccountListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::PasswordChange(PasswordChangeInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ExchangeList(ExchangeListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ExecutionReplay(ExecutionReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::LineUpdate(LineInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OpenOrderReplay(OrderReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OrderReplay(OrderReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::PnlReplay(PnlReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::PnlUpdate(PnlInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::PriceIncrUpdate(PriceIncrInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ProductRmsList(ProductRmsListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::SingleOrderReplay(SingleOrderReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::BustReport(OrderBustReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::CancelReport(OrderCancelReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::FailureReport(OrderFailureReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::FillReport(OrderFillReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ModifyReport(OrderModifyReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::NotCancelledReport(OrderNotCancelledReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::NotModifiedReport(OrderNotModifiedReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::RejectReport(OrderRejectReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::StatusReport(OrderStatusReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeCorrectReport(OrderTradeCorrectReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TriggerPulledReport(OrderTriggerPulledReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TriggerReport(OrderTriggerReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OtherReport(OrderReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::SodUpdate(SodReport*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::Quote(QuoteReport*, void*, int* aiCode) { RECV_OK }
