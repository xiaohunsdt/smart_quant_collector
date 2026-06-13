#include "binance_perpetual.h"

#include <cassert>
#include <cctype>

#include "binance_common.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace binance_perpetual {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type) {
  ParseResult result;
  try {
    switch(event_type) {
      case EventType::TICK:
        result.type = ParsedType::TICK;
        if(!binance::ParseTradeEvent(doc, result.tick, channel_id))
          result.type = ParsedType::NONE;
        else
          result.tick.exchange_timestamp *= 1000ULL;  // perpetual: ms → μs
        break;
      case EventType::DEPTH:
        // Futures partial depth uses same "depthUpdate" format
        result.type = ParsedType::DEPTH;
        if(!ParseDepthEvent(doc, result.depth, channel_id)) result.type = ParsedType::NONE;
        break;
      case EventType::BOOK_TICKER:
        result.type = ParsedType::BOOK_TICKER;
        if(!binance::ParseBookTickerEvent(doc, result.book_ticker, channel_id)) result.type = ParsedType::NONE;
        break;
      default:
        result.type = ParsedType::NONE;
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual parse: {}", e.what());
  }
  return result;
}

bool ParseDepthEvent(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    out.exchange_timestamp = static_cast<uint64_t>(doc["E"].get_int64()) * 1000ULL;
    out.last_update_id = static_cast<uint64_t>(doc["u"].get_int64());
    out.channel_id = channel_id;

    out.bid_count = 0;
    for(auto bid_level : doc["b"]) {
      if(out.bid_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = bid_level.begin();
      if(it != bid_level.end()) (void)SvToDouble((*it).get_string(), p);
      ++it;
      if(it != bid_level.end()) (void)SvToDouble((*it).get_string(), q);
      out.bids[out.bid_count++] = {p, q};
    }
    out.ask_count = 0;
    for(auto ask_level : doc["a"]) {
      if(out.ask_count >= kMaxOrderbookLevels) break;
      double p = 0.0, q = 0.0;
      auto it = ask_level.begin();
      if(it != ask_level.end()) (void)SvToDouble((*it).get_string(), p);
      ++it;
      if(it != ask_level.end()) (void)SvToDouble((*it).get_string(), q);
      out.asks[out.ask_count++] = {p, q};
    }
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual depth parse: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance perpetual depth parse: unknown error");
    return false;
  }
}

}  // namespace binance_perpetual

namespace {
static std::vector<SubscriptionGroup> BinancePerpetualBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  std::string name(symbol);
  for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  std::string dl = std::to_string(depth_level);
  // Perpetual: @bookTicker + @depth on /public, @aggTrade on /market.
  // Perpetual timestamps are in milliseconds (futures API does not support timeUnit=MICROSECOND).
  return {
      {"wss://fstream.binance.com/market/ws",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0, EventType::TICK},
       }},
      {"wss://fstream.binance.com/public/ws",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", 700, EventType::BOOK_TICKER},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth)" + dl + R"(@100ms"],"id":2})", 500, EventType::DEPTH},
       }},
  };
}
}  // namespace

const ExchangeAdapter kBinancePerpetualAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Perpetual,
    .rest_host = "fapi.binance.com",
    .build_subscribes = BinancePerpetualBuildSubscribes,
    .peek_event_type = binance::PeekEventType,
    .parse = binance_perpetual::Parse,
};

}  // namespace sqc
