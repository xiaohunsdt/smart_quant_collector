#include "gateio_spot.h"

#include <chrono>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/gateio/gateio_common.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace gateio_spot {

EventType PeekEventType(simdjson::ondemand::document& doc) {
  return gateio::PeekEventType(doc, "spot");
}

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type) {
  ParseResult result;
  try {
    switch(event_type) {
      case EventType::TICK:
        result.type = ParsedType::TICK;
        if(!ParseTradeEvent(doc, result.tick, channel_id)) result.type = ParsedType::NONE;
        break;
      case EventType::DEPTH:
        result.type = ParsedType::DEPTH;
        if(!ParseDepthEvent(doc, result.depth, channel_id)) result.type = ParsedType::NONE;
        break;
      case EventType::BOOK_TICKER:
        result.type = ParsedType::BOOK_TICKER;
        if(!ParseBookTickerEvent(doc, result.book_ticker, channel_id)) result.type = ParsedType::NONE;
        break;
      default:
        result.type = ParsedType::NONE;
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot parse: {}", e.what());
  }
  return result;
}

// ---- Trade ----

bool ParseTradeEvent(simdjson::ondemand::document& doc, TickData& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev == "subscribe") return false;
    auto result = doc["result"];
    out.trade_id = result["id"].get_uint64();
    (void)result["create_time"].get_uint64();
    double create_time_ms = 0.0;
    if (!SvToDouble(result["create_time_ms"].get_string(), create_time_ms)) return false;
    out.exchange_timestamp = static_cast<uint64_t>(create_time_ms * 1000.0);
    if (!SvToDouble(result["price"].get_string(), out.price)) return false;
    std::string_view side = result["side"].get_string();
    double qty = 0.0;
    if (!SvToDouble(result["amount"].get_string(), qty)) return false;
    out.quantity = qty < 0 ? -qty : qty;
    out.is_buyer_maker = (side == "sell");
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot trade parse: unknown error");
    return false;
  }
}

// ---- Depth (spot.order_book snapshot) ----

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    (void)doc["time"].get_uint64();
    try {
      (void)doc["time_ms"].get_uint64();
    } catch(...) {
    }
    std::string_view ev = doc["event"].get_string();
    if(ev != "update" && ev != "all") return false;
    auto result = doc["result"];
    out.exchange_timestamp = result["t"].get_uint64() * 1000ULL;
    out.last_update_id = result["lastUpdateId"].get_uint64();
    out.channel_id = channel_id;
    out.bid_count = 0;
    for(auto lv : result["bids"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if(it == lv.end()) continue;
      double p = 0.0, q = 0.0;
      if (!SvToDouble((*it).get_string(), p)) continue;
      ++it;
      if(it == lv.end()) continue;
      if (!SvToDouble((*it).get_string(), q)) continue;
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto lv : result["asks"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      auto it = lv.begin();
      if(it == lv.end()) continue;
      double p = 0.0, q = 0.0;
      if (!SvToDouble((*it).get_string(), p)) continue;
      ++it;
      if(it == lv.end()) continue;
      if (!SvToDouble((*it).get_string(), q)) continue;
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io spot depth parse: unknown error");
    return false;
  }
}

// ---- Book Ticker ---- (delegates to gateio_common)

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  return gateio::ParseBookTickerEvent(doc, out, channel_id);
}

}  // namespace gateio_spot

namespace {
static std::vector<SubscriptionGroup> GateioSpotBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  std::string dl = std::to_string(depth_level);
  return {
      {"wss://api.gateio.ws/ws/v4/",
       {
           {R"({"time":)" + ts + R"(,"channel":"spot.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0, EventType::TICK},
           {R"({"time":)" + ts + R"(,"channel":"spot.order_book","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"" + dl + "\",\"100ms\"]}", 500, EventType::DEPTH},
           {R"({"time":)" + ts + R"(,"channel":"spot.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700, EventType::BOOK_TICKER},
       }}};
}
}  // namespace

const ExchangeAdapter kGateioSpotAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioSpotBuildSubscribes,
    .peek_event_type = gateio_spot::PeekEventType,
    .parse = gateio_spot::Parse,
};

}  // namespace sqc
