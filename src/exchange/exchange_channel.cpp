#include "exchange_channel.h"

#include <boost/asio/ssl.hpp>
#include <chrono>
#include <atomic>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "simdjson.h"
#include "src/common/signal_handler.h"
#include "src/common/simdjson_utils.h"
#include "src/network/recv_buffer.h"
#include "src/network/ws_client.h"

namespace sqc {

std::string BuildGateioOrderBookSubscribePayload(
    const std::vector<SymbolSpec>& symbols,
    uint64_t timestamp_seconds) {
  std::string payload =
      R"({"time":)" + std::to_string(timestamp_seconds) +
      R"(,"channel":"futures.order_book","event":"subscribe","payload":[)";
  bool first = true;
  for (const auto& sym : symbols) {
    if (!sym.enabled) continue;
    if (!first) payload += ",";
    payload += "\"" + sym.name + "\"";
    first = false;
  }
  payload += "]}";
  return payload;
}

ExchangeChannel::~ExchangeChannel() = default;

ExchangeChannel::ExchangeChannel(
    const ChannelSpec& spec, net::io_context& ioc, net::ssl::context& ssl_ctx,
    std::vector<std::shared_ptr<ShardQueue>> shard_queues,
    const std::unordered_map<std::string, uint32_t>& symbol_to_channel_id)
    : spec_(spec), ioc_(ioc), ssl_ctx_(ssl_ctx),
      shard_queues_(std::move(shard_queues)),
      symbol_to_channel_id_(symbol_to_channel_id) {
  ws_ = std::make_unique<WsClient>(ioc_, ssl_ctx_);
  recv_buffer_ = std::make_unique<RecvBuffer>();
}

void ExchangeChannel::Start() {
  std::string url = spec_.ws_url;
  std::string host, port = "443", path = "/";
  if (url.starts_with("wss://")) url = url.substr(6);
  auto colon = url.find(':');
  auto slash = url.find('/');
  if (colon != std::string::npos && (slash == std::string::npos || colon < slash)) {
    host = url.substr(0, colon);
    port = url.substr(colon + 1, slash - colon - 1);
  } else if (slash != std::string::npos) {
    host = url.substr(0, slash);
  } else {
    host = url;
  }
  if (slash != std::string::npos) path = url.substr(slash);

  LOG_INFO(GetLogger(), "ExchangeChannel connecting to {}:{} {}", host, port, path);

  auto self = shared_from_this();
  ws_->Connect(host, port, path, [this, self](bool success) {
    if (success) {
      Subscribe();
      ws_->StartRead([this](const char* data, size_t size) { OnMessage(data, size); });
      LOG_INFO(GetLogger(), "Read loop started for {}", spec_.exchange_name);
    } else {
      LOG_WARNING(GetLogger(), "{} connect failed, retrying in 2s...", spec_.exchange_name);
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::seconds(2));
      timer->async_wait([this, self, timer](boost::system::error_code) {
        if (!SignalHandler::IsShutdownRequested()) Start();
      });
    }
  });
}

void ExchangeChannel::Stop() { if (ws_) ws_->Close(); }

void ExchangeChannel::Subscribe() {
  std::string sub_msg;
  if (spec_.exchange_name == "binance") {
    sub_msg = R"({"method":"SUBSCRIBE","params":[)";
    bool first = true;
    for (const auto& sym : spec_.symbols) {
      if (!sym.enabled) continue;
      if (!first) sub_msg += ",";
      std::string name = sym.name;
      for (auto& c : name) c = static_cast<char>(std::tolower(c));
      sub_msg += "\"" + name + "@trade\"";
      first = false;
    }
    sub_msg += R"(],"id":1})";
  } else if (spec_.exchange_name == "gateio") {
    sub_msg = R"({"time":)" +
              std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count()) +
              R"(,"channel":"futures.tickers","event":"subscribe","payload":[)";
    bool first = true;
    for (const auto& sym : spec_.symbols) {
      if (!sym.enabled) continue;
      if (!first) sub_msg += ",";
      sub_msg += "\"" + sym.name + "\"";
      first = false;
    }
    sub_msg += "]}";
  }
  LOG_INFO(GetLogger(), "Sending subscribe: {}", sub_msg);
  ws_->Write(sub_msg);

  // Chain order_book subscribe after tickers subscribe completes
  if (spec_.exchange_name == "gateio") {
    auto depth_sub = std::make_shared<std::string>(
        BuildGateioOrderBookSubscribePayload(
          spec_.symbols,
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

    auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::milliseconds(500));
    timer->async_wait([this, depth_sub, timer](boost::system::error_code) {
      LOG_INFO(GetLogger(), "Sending depth subscribe: {}", *depth_sub);
      ws_->Write(*depth_sub);
    });
  }
}

void ExchangeChannel::OnMessage(const char* data, size_t size) {
  static std::atomic<int> msg_count{0};
  int n = ++msg_count;
  if (size == 0 || shard_queues_.empty()) return;

  // Allocate with trailing simdjson padding (parser reads past JSON end)
  std::shared_ptr<char[]> json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), data, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  // Peek parse to (1) filter control messages and (2) extract symbol -> channel_id
  uint32_t channel_id = 0;
  simdjson::ondemand::parser peek_parser;
  simdjson::ondemand::document peek_doc;
  auto err = peek_parser.iterate(json_data.get(), size, size + kSimdjsonPadding).get(peek_doc);
  if (!err) {
    try {
      // CRITICAL: simdjson ondemand is forward-only — field access MUST match
      // JSON field order. Gate.io: time, time_ms, channel, event, result
      std::string_view symbol;
      if (spec_.exchange_name == "gateio") {
        std::string_view channel = peek_doc["channel"];
        std::string_view event = peek_doc["event"];
        if (event == "subscribe" || event == "unsubscribe") {
          LOG_INFO(GetLogger(), "OnMessage #{} received control message: {}", n, event);
          return;
        }
        // futures.tickers → result is array; futures.order_book → result is object
        auto result = peek_doc["result"];
        if (channel == "futures.tickers") {
          for (auto item : result.get_array()) {
            symbol = std::string_view(item["contract"]);
            break;
          }
        } else {
          symbol = std::string_view(result["contract"]);
        }
      } else {
        // Binance
        std::string_view etype;
        try { etype = std::string_view(peek_doc["e"]); }
        catch (...) { return; }
        symbol = std::string_view(peek_doc["s"]);
      }
      auto key = spec_.exchange_name + ":" + spec_.channel_type + ":" + std::string(symbol);
      auto it = symbol_to_channel_id_.find(key);
      if (it != symbol_to_channel_id_.end()) channel_id = it->second;
    } catch (...) { channel_id = 0; }
  }

  RawMessage msg;
  msg.data = json_data;
  msg.size = size;
  msg.channel_id = channel_id;
  msg.exchange = spec_.exchange_name;

  uint32_t shard = (channel_id ^ std::hash<std::string>{}(spec_.exchange_name)) % shard_queues_.size();
  shard_queues_[shard]->Push(std::move(msg));
}

uint32_t ExchangeChannel::ComputeShardIndex(std::string_view symbol) const {
  if (shard_queues_.empty()) return 0;
  return static_cast<uint32_t>(std::hash<std::string_view>{}(symbol)) % shard_queues_.size();
}

}  // namespace sqc
