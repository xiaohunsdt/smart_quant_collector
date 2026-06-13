#include "rithmic_callbacks.h"

#include <chrono>

#include "quill/LogMacros.h"

using namespace sqc::rithmic;

// ============================================================================
// Global login state atomics (exactly matching rithmic_md_saver pattern)
// ============================================================================

std::atomic<int> g_iRepLoginStatus{LoginStatus_NotLoggedIn};
std::atomic<bool> g_bRcvdUnacceptedAgreements{false};
std::atomic<int> g_iUnacceptedMandatoryAgreements{0};
std::atomic<int> g_iMdLoginStatus{LoginStatus_NotLoggedIn};

// ============================================================================
// RECV_OK — every callback must set *aiCode = API_OK and return OK
// ============================================================================

#define RECV_OK     \
  *aiCode = API_OK; \
  return (OK);

// ============================================================================
// Helpers
// ============================================================================

namespace {

std::string_view ToSv(const tsNCharcb& s) noexcept {
  if(!s.pData) return {};
  auto sv = std::string_view(s.pData, static_cast<size_t>(s.iDataLen));
  while(!sv.empty() && sv.back() == ' ') sv.remove_suffix(1);
  return sv;
}

void CopySymbol(char dst[12], std::string_view src) noexcept {
  size_t n = src.size() < 11 ? src.size() : 11;
  std::memcpy(dst, src.data(), n);
  dst[n] = '\0';
}

}  // namespace

// ============================================================================
// MyCallbacks — helpers
// ============================================================================

sqc::rithmic::RithmicOrderBook* MyCallbacks::FindBook(std::string_view exchange, std::string_view ticker) {
  uint32_t channel_id = m_channelMap.Lookup(exchange, ticker);
  if(channel_id == sqc::rithmic::RithmicChannelMap::kNotFound) return nullptr;

  auto it = m_channelToBook.find(channel_id);
  if(it != m_channelToBook.end()) return &m_books[it->second];

  // Slow path: allocate a new book slot (startup only)
  if(m_bookCount >= kMaxBooks) {
    LOG_ERROR(logger, "OrderBook limit ({}) exceeded for ch={} {}/{}", kMaxBooks, channel_id, exchange, ticker);
    return nullptr;
  }
  uint8_t idx = static_cast<uint8_t>(m_bookCount);
  m_books[idx].Init(channel_id, ticker.data());
  m_channelToBook[channel_id] = idx;
  ++m_bookCount;

  LOG_INFO(logger, "OrderBook created: ch={} sym={} (slot {})", channel_id, ticker, idx);
  return &m_books[idx];
}

void MyCallbacks::PushSnapshot(sqc::rithmic::RithmicOrderBook& book, uint64_t exchange_ts) {
  sqc::DepthUpdateEvent depth{};
  book.SnapshotTo(depth, exchange_ts);
  depth.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  (void)m_depthQueue.TryPush(depth);
}

// ============================================================================
// MyAdmCallbacks
// ============================================================================

int MyAdmCallbacks::Alert(AlertInfo* pInfo, void* pContext, int* aiCode) {
  int iIgnored;
  pInfo->dump(&iIgnored);
  RECV_OK
}

// ============================================================================
// MyCallbacks — constructor
// ============================================================================

MyCallbacks::MyCallbacks(sqc::rithmic::shm_layout::TickQueue& tick_queue, sqc::rithmic::shm_layout::DepthQueue& depth_queue,
                         sqc::rithmic::shm_layout::BookTickerQueue& book_ticker_queue, const sqc::rithmic::RithmicChannelMap& channel_map,
                         sqc::rithmic::SsboeConverter& converter, uint32_t max_depth_levels)
    : m_tickQueue(tick_queue),
      m_depthQueue(depth_queue),
      m_bookTickerQueue(book_ticker_queue),
      m_channelMap(channel_map),
      m_converter(converter),
      m_maxDepthLevels(max_depth_levels) {
  // m_channelToBook is an unordered_map; empty = no books mapped yet.
}

// ============================================================================
// Login / agreement callbacks
// ============================================================================

int MyCallbacks::Alert(AlertInfo* pInfo, void* pContext, int* aiCode) {
  int iIgnored;
  pInfo->dump(&iIgnored);

  if(pInfo->iConnectionId == REPOSITORY_CONNECTION_ID) {
    if(pInfo->iAlertType == ALERT_LOGIN_COMPLETE)
      g_iRepLoginStatus = LoginStatus_Complete;
    else if(pInfo->iAlertType == ALERT_LOGIN_FAILED || pInfo->iAlertType == ALERT_CONNECTION_BROKEN)
      g_iRepLoginStatus = LoginStatus_Failed;
  }

  if(pInfo->iConnectionId == MARKET_DATA_CONNECTION_ID) {
    if(pInfo->iAlertType == ALERT_LOGIN_COMPLETE)
      g_iMdLoginStatus = LoginStatus_Complete;
    else if(pInfo->iAlertType == ALERT_LOGIN_FAILED || pInfo->iAlertType == ALERT_CONNECTION_BROKEN)
      g_iMdLoginStatus = LoginStatus_Failed;
  }

  RECV_OK
}

int MyCallbacks::AgreementList(AgreementListInfo* pInfo, void* pContext, int* aiCode) {
  int iIgnored;
  pInfo->dump(&iIgnored);

  if(!pInfo->bAccepted) {
    tsNCharcb sActive = {const_cast<char*>("active"), 6};
    for(int i = 0; i < pInfo->iArrayLen; i++) {
      AgreementInfo& oAg = pInfo->asAgreementInfoArray[i];
      if(oAg.sStatus.iDataLen == sActive.iDataLen && memcmp(oAg.sStatus.pData, sActive.pData, oAg.sStatus.iDataLen) == 0) {
        if(oAg.bMandatory) {
          g_iUnacceptedMandatoryAgreements++;
        }
      }
    }
    g_bRcvdUnacceptedAgreements = true;
  }

  RECV_OK
}

// ============================================================================
// Hot-path: TradePrint → TickData (tick queue)
// ============================================================================

int MyCallbacks::TradePrint(TradeInfo* pInfo, void* pContext, int* aiCode) {
  sqc::TickData tick{};
  tick.channel_id = m_channelMap.Lookup(ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
  if(tick.channel_id == sqc::rithmic::RithmicChannelMap::kNotFound) {
    LOG_WARNING(logger, "TradePrint: UNMAPPED exch={} ticker={} — dropping", ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
    RECV_OK
  }

  tick.exchange_timestamp = m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs);
  tick.price = pInfo->dPrice;
  tick.quantity = static_cast<double>(pInfo->llSize);
  tick.trade_id = 0;
  CopySymbol(tick.symbol, ToSv(pInfo->sTicker));
  tick.is_buyer_maker = (ToSv(pInfo->sAggressorSide) == "S");
  tick.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

  (void)m_tickQueue.TryPush(tick);

  // int iIgnored;
  // pInfo->dump(&iIgnored);

  RECV_OK
}

// ============================================================================
// Hot-path: BestBidAskQuote → BookTickerEvent (book ticker queue)
// ============================================================================

int MyCallbacks::BestBidAskQuote(BidInfo* pBid, AskInfo* pAsk, void* pContext, int* aiCode) {
  sqc::BookTickerEvent bt{};
  bt.channel_id = m_channelMap.Lookup(ToSv(pBid->sExchange), ToSv(pBid->sTicker));
  if(bt.channel_id == sqc::rithmic::RithmicChannelMap::kNotFound) {
    RECV_OK
  }

  bt.exchange_timestamp = m_converter.ToEpochMicros(pBid->iSsboe, pBid->iUsecs);
  CopySymbol(bt.symbol, ToSv(pBid->sTicker));
  bt.best_bid_price = pBid->bPriceFlag ? pBid->dPrice : 0.0;
  bt.best_bid_qty = pBid->bSizeFlag ? static_cast<double>(pBid->llSize) : 0.0;
  bt.best_ask_price = pAsk->bPriceFlag ? pAsk->dPrice : 0.0;
  bt.best_ask_qty = pAsk->bSizeFlag ? static_cast<double>(pAsk->llSize) : 0.0;
  bt.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

  (void)m_bookTickerQueue.TryPush(bt);

  // int iIgnored;
  // pBid->dump(&iIgnored);
  // pAsk->dump(&iIgnored);

  RECV_OK
}

// ============================================================================
// Hot-path: LimitOrderBook → DepthUpdateEvent (depth queue)
// ============================================================================

int MyCallbacks::LimitOrderBook(LimitOrderBookInfo* pInfo, void* pContext, int* aiCode) {
  auto* book = FindBook(ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
  if(!book) {
    LOG_WARNING(logger, "LimitOrderBook: UNMAPPED exch={} ticker={} — dropping", ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
    int clear_code = API_OK;
    pInfo->clearHandles(&clear_code);
    RECV_OK
  }

  uint64_t ts = m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs);

  book->Lock();

  if(pInfo->iType == kMdImageCb) {
    // Full snapshot: replace entire book
    LOG_INFO(logger, "LimitOrderBook IMAGE: exch={} ticker={} ask_len={} bid_len={} ch={}", ToSv(pInfo->sExchange), ToSv(pInfo->sTicker),
             pInfo->iAskArrayLen, pInfo->iBidArrayLen, book->channel_id());

    book->ApplyImage(pInfo->adAskPriceArray, pInfo->allAskSizeArray, pInfo->iAskArrayLen, /*is_ask=*/true);
    book->ApplyImage(pInfo->adBidPriceArray, pInfo->allBidSizeArray, pInfo->iBidArrayLen, /*is_ask=*/false);
  } else {
    // MD_UPDATE_CB / MD_HISTORY_CB: incremental multi-level update
    LOG_INFO(logger, "LimitOrderBook UPDATE: exch={} ticker={} iType={} ask_len={} bid_len={} ch={}", ToSv(pInfo->sExchange), ToSv(pInfo->sTicker),
             pInfo->iType, pInfo->iAskArrayLen, pInfo->iBidArrayLen, book->channel_id());

    uint32_t maxd = m_maxDepthLevels;
    if(maxd > sqc::kMaxOrderbookLevels) maxd = sqc::kMaxOrderbookLevels;

    uint32_t ask_n = static_cast<uint32_t>(pInfo->iAskArrayLen);
    if(ask_n > maxd) ask_n = maxd;
    for(uint32_t i = 0; i < ask_n; ++i) {
      book->ApplyLevel(pInfo->adAskPriceArray[i], static_cast<double>(pInfo->allAskSizeArray[i]),
                       /*is_ask=*/true);
    }

    uint32_t bid_n = static_cast<uint32_t>(pInfo->iBidArrayLen);
    if(bid_n > maxd) bid_n = maxd;
    for(uint32_t i = 0; i < bid_n; ++i) {
      book->ApplyLevel(pInfo->adBidPriceArray[i], static_cast<double>(pInfo->allBidSizeArray[i]),
                       /*is_ask=*/false);
    }
  }

  PushSnapshot(*book, ts);
  book->Unlock();

  // int clear_code = API_OK;
  // pInfo->clearHandles(&clear_code);
  RECV_OK
}

// ============================================================================
// TradeCondition — forward as tick
// ============================================================================

int MyCallbacks::TradeCondition(TradeInfo* pInfo, void* pContext, int* aiCode) { return TradePrint(pInfo, pContext, aiCode); }

// ============================================================================
// All other callbacks — stub with RECV_OK
// ============================================================================

int MyCallbacks::AskQuote(AskInfo* pInfo, void*, int* aiCode) {
  auto* book = FindBook(ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
  if(!book) {
    RECV_OK
  }

  book->Lock();

  switch(pInfo->iUpdateType) {
    case kUpdateTypeSolo:
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->ApplyLevel(pInfo->dPrice, qty, /*is_ask=*/true);
      }
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    case kUpdateTypeBegin:
      if(book->batch_active()) book->CancelBatch();
      book->BeginBatch();
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/true);
      }
      break;
    case kUpdateTypeMiddle:
      if(!book->batch_active()) book->BeginBatch();
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/true);
      }
      break;
    case kUpdateTypeEnd:
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/true);
      }
      book->CommitBatch();
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    case kUpdateTypeClear:
      book->Clear();
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    default:
      break;
  }

  book->Unlock();
  RECV_OK
}
int MyCallbacks::BestAskQuote(AskInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::BestBidQuote(BidInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::BidQuote(BidInfo* pInfo, void*, int* aiCode) {
  auto* book = FindBook(ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
  if(!book) {
    RECV_OK
  }

  book->Lock();

  switch(pInfo->iUpdateType) {
    case kUpdateTypeSolo:
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->ApplyLevel(pInfo->dPrice, qty, /*is_ask=*/false);
      }
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    case kUpdateTypeBegin:
      if(book->batch_active()) book->CancelBatch();
      book->BeginBatch();
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/false);
      }
      break;
    case kUpdateTypeMiddle:
      if(!book->batch_active()) book->BeginBatch();
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/false);
      }
      break;
    case kUpdateTypeEnd:
      if(pInfo->bPriceFlag) {
        double qty = pInfo->bSizeFlag ? static_cast<double>(pInfo->llSize) : 0.0;
        book->BufferLevel(pInfo->dPrice, qty, /*is_ask=*/false);
      }
      book->CommitBatch();
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    case kUpdateTypeClear:
      book->Clear();
      PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
      break;
    default:
      break;
  }

  book->Unlock();
  RECV_OK
}
int MyCallbacks::BinaryContractList(BinaryContractListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ClosePrice(ClosePriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::ClosingIndicator(ClosingIndicatorInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::EndQuote(EndQuoteInfo* pInfo, void*, int* aiCode) {
  auto* book = FindBook(ToSv(pInfo->sExchange), ToSv(pInfo->sTicker));
  if(!book) {
    RECV_OK
  }

  book->Lock();
  if(book->batch_active()) {
    book->CommitBatch();
    PushSnapshot(*book, m_converter.ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs));
  }
  book->Unlock();
  RECV_OK
}
int MyCallbacks::EquityOptionStrategyList(EquityOptionStrategyListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::HighPrice(HighPriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::InstrumentByUnderlying(InstrumentByUnderlyingInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::InstrumentSearch(InstrumentSearchInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::LowPrice(LowPriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::MarketMode(MarketModeInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::MidPrice(MidPriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OpenInterest(OpenInterestInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OpenPrice(OpenPriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OpeningIndicator(OpeningIndicatorInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::OptionList(OptionListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::RefData(RefDataInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::SettlementPrice(SettlementPriceInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::Strategy(StrategyInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::StrategyList(StrategyListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeReplay(TradeReplayInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeRoute(TradeRouteInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeRouteList(TradeRouteListInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::TradeVolume(TradeVolumeInfo*, void*, int* aiCode) { RECV_OK }
int MyCallbacks::Bar(BarInfo*, void*, int* aiCode) { RECV_OK }
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
