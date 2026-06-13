#include "gateio_perpetual.h"

#include <chrono>

#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/gateio/gateio_common.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace gateio_perpetual {

EventType PeekEventType(simdjson::ondemand::document& doc) {
  return gateio::PeekEventType(doc, "futures");
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
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual parse: {}", e.what());
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
    auto arr = doc["result"].get_array();
    auto it = arr.begin();
    if(it == arr.end()) return false;
    auto item = *it;
    out.trade_id = item["id"].get_uint64();
    (void)item["create_time"].get_uint64();
    out.exchange_timestamp = item["create_time_ms"].get_uint64() * 1000ULL;
    if (!SvToDouble(item["price"].get_string(), out.price)) return false;
    double size_val = 0.0;
    std::string_view size_sv;
    if (item["size"].get_string().get(size_sv) == simdjson::SUCCESS)
      SvToDouble(size_sv, size_val);
    else
      size_val = static_cast<double>(item["size"].get_int64());
    out.quantity = size_val < 0 ? -size_val : size_val;
    out.is_buyer_maker = (size_val < 0);
    out.channel_id = channel_id;
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual trade parse: unknown error");
    return false;
  }
}

// ---- Depth (futures.order_book snapshot) ----

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
    out.last_update_id = result["id"].get_uint64();
    out.channel_id = channel_id;
    out.bid_count = 0;
    for(auto lv : result["bids"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      if (!SvToDouble(lv["p"].get_string(), p)) continue;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) SvToDouble(s_str.value_unsafe(), q);
      else q = static_cast<double>(lv["s"].get_int64());
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto lv : result["asks"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      if (!SvToDouble(lv["p"].get_string(), p)) continue;
      auto s_str = lv["s"].get_string();
      if (s_str.error() == simdjson::SUCCESS) SvToDouble(s_str.value_unsafe(), q);
      else q = static_cast<double>(lv["s"].get_int64());
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* l = GetLogger()) LOG_ERROR(l, "Gate.io perpetual depth parse: unknown error");
    return false;
  }
}

// ---- Book Ticker ---- (delegates to gateio_common)

bool ParseBookTickerEvent(simdjson::ondemand::document& doc, BookTickerEvent& out, uint32_t channel_id) {
  return gateio::ParseBookTickerEvent(doc, out, channel_id);
}

}  // namespace gateio_perpetual

namespace {
static std::vector<SubscriptionGroup> GateioPerpetualBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::string ts = std::to_string(now_sec);
  std::string dl = std::to_string(depth_level);
  return {{"wss://fx-ws.gateio.ws/v4/ws/usdt",
           {
               {R"({"time":)" + ts + R"(,"channel":"futures.trades","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 0, EventType::TICK},
               {R"({"time":)" + ts + R"(,"channel":"futures.order_book","event":"subscribe","payload":[")" + std::string(symbol) + "\",\"" + dl + "\",\"0\"]}", 500, EventType::DEPTH},
               {R"({"time":)" + ts + R"(,"channel":"futures.book_ticker","event":"subscribe","payload":[")" + std::string(symbol) + R"("]})", 700, EventType::BOOK_TICKER},
           }}};
}
}  // namespace

const ExchangeAdapter kGateioPerpetualAdapter = {
    .name = "gateio",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "api.gateio.ws",
    .build_subscribes = GateioPerpetualBuildSubscribes,
    .peek_event_type = gateio_perpetual::PeekEventType,
    .parse = gateio_perpetual::Parse,
};

}  // namespace sqc
