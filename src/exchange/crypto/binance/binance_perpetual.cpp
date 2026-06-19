#include "binance_perpetual.h"

#include <cassert>
#include <cctype>

#include "binance_common.h"
#include "common/logger_init.h"
#include "common/string_utils.h"
#include "exchange/crypto/json_parse_helpers.h"
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
          result.tick.exchange_timestamp = MsToUs(result.tick.exchange_timestamp);  // perpetual: ms → μs
        break;
      case EventType::DEPTH:
        // Futures partial depth uses same "depthUpdate" format
        DispatchParse(result, ParsedType::DEPTH, [&] { return ParseDepthEvent(doc, result.depth, channel_id); });
        break;
      case EventType::BOOK_TICKER:
        DispatchParse(result, ParsedType::BOOK_TICKER,
                      [&] { return binance::ParseBookTickerEvent(doc, result.book_ticker, channel_id); });
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
    out.exchange_timestamp = MsToUs(static_cast<uint64_t>(doc["E"].get_int64()));
    out.last_update_id = static_cast<uint64_t>(doc["u"].get_int64());
    out.channel_id = channel_id;

    out.bid_count = ParsePositionalLevelArray(doc["b"].get_array(), out.bids, kMaxOrderbookLevels);
    out.ask_count = ParsePositionalLevelArray(doc["a"].get_array(), out.asks, kMaxOrderbookLevels);
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
  for(auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
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
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@bookTicker"],"id":3})", kBookTickerSubscribeDelayMs, EventType::BOOK_TICKER},
           {R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth)" + dl + R"(@100ms"],"id":2})", kDepthSubscribeDelayMs, EventType::DEPTH},
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
