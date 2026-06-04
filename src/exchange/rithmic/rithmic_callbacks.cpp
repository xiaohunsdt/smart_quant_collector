#include "src/exchange/rithmic/rithmic_callbacks.h"

#include <chrono>
#include <cstring>

#include "RApiPlus.h"

namespace sqc {
namespace rithmic {

// ============================================================================
// Helper: extract tsNCharcb as string_view
// ============================================================================
namespace {

std::string_view ToSv(const tsNCharcb& s) noexcept {
  if (!s.pData) return {};
  auto sv = std::string_view(s.pData, static_cast<size_t>(s.iDataLen));
  while (!sv.empty() && sv.back() == ' ') sv.remove_suffix(1);
  return sv;
}

void CopySymbol(char dst[12], std::string_view src) noexcept {
  size_t n = src.size() < 11 ? src.size() : 11;
  std::memcpy(dst, src.data(), n);
  dst[n] = '\0';
}

}  // namespace

// ============================================================================
// RCallbacksImpl — overrides only hot-path callbacks
// ============================================================================

namespace {

class RCallbacksImpl final : public RApi::RCallbacks {
 public:
  RithmicCallbacks* owner = nullptr;

  int Alert(RApi::AlertInfo* pInfo, void*, int* aiCode) override {
    if (!owner) { *aiCode = API_OK; return OK; }

    // Login status tracking
    if (pInfo->iConnectionId == RApi::REPOSITORY_CONNECTION_ID) {
      if (pInfo->iAlertType == RApi::ALERT_LOGIN_COMPLETE) {
        owner->rep_login_status().store(kLoginComplete, std::memory_order_release);
      } else if (pInfo->iAlertType == RApi::ALERT_LOGIN_FAILED) {
        owner->rep_login_status().store(kLoginFailed, std::memory_order_release);
      }
    }
    if (pInfo->iConnectionId == RApi::MARKET_DATA_CONNECTION_ID) {
      if (pInfo->iAlertType == RApi::ALERT_LOGIN_COMPLETE) {
        owner->md_login_status().store(kLoginComplete, std::memory_order_release);
      } else if (pInfo->iAlertType == RApi::ALERT_LOGIN_FAILED) {
        owner->md_login_status().store(kLoginFailed, std::memory_order_release);
      }
    }
    *aiCode = API_OK;
    return OK;
  }

  // ---- Hot-path: TradePrint → TickData ----

  int TradePrint(RApi::TradeInfo* pInfo, void*, int* aiCode) override {
    if (!owner) { *aiCode = API_OK; return OK; }

    RithmicEvent event;
    event.type = EventType::TICK;

    auto tk_sv = ToSv(pInfo->sTicker);
    event.channel_id = owner->channel_map().Lookup("", tk_sv);
    if (event.channel_id == RithmicChannelMap::kNotFound) {
      *aiCode = API_OK;
      return OK;  // symbol not configured — skip
    }

    auto& tick = event.tick;
    tick.exchange_timestamp = owner->converter().ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs);
    tick.price = pInfo->dPrice;
    tick.quantity = static_cast<double>(pInfo->llSize);
    tick.trade_id = 0;  // Rithmic TradeInfo has no numeric trade ID
    tick.channel_id = event.channel_id;
    CopySymbol(tick.symbol, tk_sv);
    tick.is_buyer_maker = (ToSv(pInfo->sAggressorSide) == "S");
    std::memset(tick.padding, 0, sizeof(tick.padding));
    tick.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    (void)owner->queue().TryPush(event);
    *aiCode = API_OK;
    return OK;
  }

  // ---- Hot-path: BestBidAskQuote → BookTickerEvent ----

  int BestBidAskQuote(RApi::BidInfo* pBid, RApi::AskInfo* pAsk, void*, int* aiCode) override {
    if (!owner) { *aiCode = API_OK; return OK; }

    RithmicEvent event;
    event.type = EventType::BOOK_TICKER;

    auto tk_sv = ToSv(pBid->sTicker);
    event.channel_id = owner->channel_map().Lookup("", tk_sv);
    if (event.channel_id == RithmicChannelMap::kNotFound) {
      *aiCode = API_OK;
      return OK;
    }

    auto& bt = event.book_ticker;
    bt.channel_id = event.channel_id;
    bt.exchange_timestamp = owner->converter().ToEpochMicros(pBid->iSsboe, pBid->iUsecs);
    CopySymbol(bt.symbol, tk_sv);
    bt.best_bid_price = pBid->bPriceFlag ? pBid->dPrice : 0.0;
    bt.best_bid_qty = pBid->bSizeFlag ? static_cast<double>(pBid->llSize) : 0.0;
    bt.best_ask_price = pAsk->bPriceFlag ? pAsk->dPrice : 0.0;
    bt.best_ask_qty = pAsk->bSizeFlag ? static_cast<double>(pAsk->llSize) : 0.0;
    bt.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    (void)owner->queue().TryPush(event);
    *aiCode = API_OK;
    return OK;
  }

  // ---- BidQuote / AskQuote: forward as individual price level updates ----
  // These are supplementary to LimitOrderBook (full snapshots).
  // For MVP we forward as best-quote BookTickerEvents since each
  // individual quote update has price+size at a single level.

  int BidQuote(RApi::BidInfo* pInfo, void*, int* aiCode) override {
    // Individual quote updates arrive at high frequency.
    // For now, skip — LimitOrderBook provides full snapshots.
    (void)pInfo;
    *aiCode = API_OK;
    return OK;
  }

  int AskQuote(RApi::AskInfo* pInfo, void*, int* aiCode) override {
    (void)pInfo;
    *aiCode = API_OK;
    return OK;
  }

  int BestAskQuote(RApi::AskInfo* pInfo, void*, int* aiCode) override {
    (void)pInfo;
    *aiCode = API_OK;
    return OK;
  }

  int BestBidQuote(RApi::BidInfo* pInfo, void*, int* aiCode) override {
    (void)pInfo;
    *aiCode = API_OK;
    return OK;
  }

  // ---- Hot-path: LimitOrderBook → DepthUpdateEvent ----

  int LimitOrderBook(RApi::LimitOrderBookInfo* pInfo, void*, int* aiCode) override {
    if (!owner) { *aiCode = API_OK; return OK; }

    RithmicEvent event;
    event.type = EventType::DEPTH;

    auto tk_sv = ToSv(pInfo->sTicker);
    event.channel_id = owner->channel_map().Lookup("", tk_sv);
    if (event.channel_id == RithmicChannelMap::kNotFound) {
      *aiCode = API_OK;
      return OK;
    }

    auto& depth = event.depth;
    depth.channel_id = event.channel_id;
    depth.exchange_timestamp = owner->converter().ToEpochMicros(pInfo->iSsboe, pInfo->iUsecs);
    CopySymbol(depth.symbol, tk_sv);

    // Copy asks (ascending price order — lowest ask first)
    uint32_t max_depth = owner->max_depth_levels();
    uint32_t ask_count = static_cast<uint32_t>(pInfo->iAskArrayLen);
    if (ask_count > max_depth) ask_count = max_depth;
    if (ask_count > kMaxOrderbookLevels) ask_count = kMaxOrderbookLevels;

    for (uint32_t i = 0; i < ask_count; ++i) {
      depth.asks[i].price = pInfo->adAskPriceArray[i];
      depth.asks[i].quantity = static_cast<double>(pInfo->allAskSizeArray[i]);
    }
    depth.ask_count = ask_count;

    // Copy bids (descending price order — highest bid first)
    uint32_t bid_count = static_cast<uint32_t>(pInfo->iBidArrayLen);
    if (bid_count > max_depth) bid_count = max_depth;
    if (bid_count > kMaxOrderbookLevels) bid_count = kMaxOrderbookLevels;

    for (uint32_t i = 0; i < bid_count; ++i) {
      depth.bids[i].price = pInfo->adBidPriceArray[i];
      depth.bids[i].quantity = static_cast<double>(pInfo->allBidSizeArray[i]);
    }
    depth.bid_count = bid_count;

    // CRITICAL: must free RApi-allocated arrays
    int clear_code = API_OK;
    pInfo->clearHandles(&clear_code);
    depth.local_diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    (void)owner->queue().TryPush(event);
    *aiCode = API_OK;
    return OK;
  }

  // ---- TradeCondition: also forward as tick ----

  int TradeCondition(RApi::TradeInfo* pInfo, void*, int* aiCode) override {
    // Conditional trades (e.g., spread trades) — forward same as regular trades
    return TradePrint(pInfo, nullptr, aiCode);
  }

  // All other ~85 callbacks use RApi::RCallbacks default implementations (return OK).
};

// ============================================================================
// AdmCallbacksImpl — overrides Alert only
// ============================================================================

class AdmCallbacksImpl final : public RApi::AdmCallbacks {
 public:
  RithmicCallbacks* owner = nullptr;

  int Alert(RApi::AlertInfo* pInfo, void*, int* aiCode) override {
    if (!owner) { *aiCode = API_OK; return OK; }
    if (pInfo->iConnectionId == RApi::REPOSITORY_CONNECTION_ID) {
      if (pInfo->iAlertType == RApi::ALERT_LOGIN_COMPLETE) {
        owner->rep_login_status().store(kLoginComplete, std::memory_order_release);
      } else if (pInfo->iAlertType == RApi::ALERT_LOGIN_FAILED) {
        owner->rep_login_status().store(kLoginFailed, std::memory_order_release);
      }
    }
    *aiCode = API_OK;
    return OK;
  }
};

}  // namespace

// ============================================================================
// RithmicCallbacks Constructor / Destructor
// ============================================================================

RithmicCallbacks::RithmicCallbacks(MpscRithmicQueue<>& queue,
                                     const RithmicChannelMap& channel_map,
                                     SsboeConverter& converter,
                                     std::atomic<int>& rep_login_status,
                                     std::atomic<int>& md_login_status,
                                     uint32_t max_depth_levels)
    : queue_(queue),
      channel_map_(channel_map),
      converter_(converter),
      rep_login_status_(rep_login_status),
      md_login_status_(md_login_status),
      max_depth_levels_(max_depth_levels) {
  auto* r = new RCallbacksImpl();
  r->owner = this;
  r_callbacks_.reset(r);

  auto* a = new AdmCallbacksImpl();
  a->owner = this;
  adm_callbacks_.reset(a);
}

RithmicCallbacks::~RithmicCallbacks() {
  // Unique_ptr handles deletion. Order matters: r_callbacks_ and adm_callbacks_
  // must be destroyed before the refs they access (queue_, channel_map_, etc.).
  r_callbacks_.reset();
  adm_callbacks_.reset();
}

}  // namespace rithmic
}  // namespace sqc
