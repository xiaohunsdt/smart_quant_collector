#include "binance_spot.h"

#include <cassert>
#include <cctype>
#include <chrono>

#include "binance_common.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/json_parse_helpers.h"
#include "quill/LogMacros.h"

namespace sqc {
namespace binance_spot {

ParseResult Parse(simdjson::ondemand::document& doc, uint32_t channel_id, EventType event_type) {
  ParseResult result;
  try {
    switch(event_type) {
      case EventType::TICK:
        DispatchParse(result, ParsedType::TICK, [&] { return binance::ParseTradeEvent(doc, result.tick, channel_id); });
        break;
      case EventType::DEPTH:
        // Spot uses partial depth format (no "e", "bids"/"asks", "lastUpdateId")
        DispatchParse(result, ParsedType::DEPTH, [&] { return ParsePartialDepth(doc, result.depth, channel_id); });
        break;
      case EventType::BOOK_TICKER:
        // Spot @bookTicker omits "e" and "E" fields
        doc.rewind();
        result.type = ParsedType::BOOK_TICKER;
        if(!binance::ParseSpotBookTickerEvent(doc, result.book_ticker, channel_id)) result.type = ParsedType::NONE;
        break;
      default:
        result.type = ParsedType::NONE;
        break;
    }
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance spot parse: {}", e.what());
  }
  return result;
}

// Parse spot partial book depth: @depth<levels>@100ms
// Format: {"lastUpdateId": N, "bids": [["price","qty",[]]], "asks": [["price","qty",[]]]}
bool ParsePartialDepth(simdjson::ondemand::document& doc, DepthUpdateEvent& out, uint32_t channel_id) {
  try {
    out.last_update_id = static_cast<uint64_t>(doc["lastUpdateId"].get_uint64());
    out.exchange_timestamp =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    out.channel_id = channel_id;

    out.bid_count = ParsePositionalLevelArray(doc["bids"].get_array(), out.bids, kMaxOrderbookLevels);
    out.ask_count = ParsePositionalLevelArray(doc["asks"].get_array(), out.asks, kMaxOrderbookLevels);
    return true;
  } catch(const simdjson::simdjson_error& e) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance spot partial depth: {}", e.what());
    return false;
  } catch(...) {
    if(auto* log = GetLogger()) LOG_ERROR(log, "Binance spot partial depth: unknown error");
    return false;
  }
}

}  // namespace binance_spot

namespace {
static std::vector<SubscriptionGroup> BinanceSpotBuildSubscribes(std::string_view symbol, uint32_t depth_level) {
  std::string name(symbol);
  for(auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return {
      {"wss://stream.binance.com/ws?timeUnit=MICROSECOND",
       {
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@aggTrade"],"id":1})", 0, EventType::TICK},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth)" + std::to_string(depth_level) + R"(@100ms"],"id":2})", kDepthSubscribeDelayMs, EventType::DEPTH},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", kBookTickerSubscribeDelayMs, EventType::BOOK_TICKER},
       }}};
}
}  // namespace

const ExchangeAdapter kBinanceSpotAdapter = {
    .name = "binance",
    .channel_type = ChannelType::Spot,
    .rest_host = "api.binance.com",
    .build_subscribes = BinanceSpotBuildSubscribes,
    .peek_event_type = binance::PeekEventType,
    .parse = binance_spot::Parse,
};

}  // namespace sqc
