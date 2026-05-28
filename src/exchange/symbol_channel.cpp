#include "symbol_channel.h"

#include <boost/asio/ssl.hpp>
#include <chrono>

#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "src/common/signal_handler.h"
#include "src/common/simdjson_utils.h"
#include "src/network/ws_client.h"

namespace sqc {

SymbolChannel::SymbolChannel(
    std::string exchange_name, std::string channel_type,
    std::string ws_url, std::string rest_host,
    std::string symbol, uint32_t depth_level,
    uint32_t channel_id,
    net::io_context& ioc, net::ssl::context& ssl_ctx,
    std::vector<std::shared_ptr<ShardQueue>> shard_queues)
    : exchange_name_(std::move(exchange_name)),
      channel_type_(std::move(channel_type)),
      ws_url_(std::move(ws_url)),
      rest_host_(std::move(rest_host)),
      symbol_(std::move(symbol)),
      depth_level_(depth_level),
      channel_id_(channel_id),
      ioc_(ioc), ssl_ctx_(ssl_ctx),
      shard_queues_(std::move(shard_queues)) {
  ws_ = std::make_unique<WsClient>(ioc_, ssl_ctx_);
}

SymbolChannel::~SymbolChannel() = default;

void SymbolChannel::Start() {
  std::string url = ws_url_;
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

  LOG_INFO(GetLogger(), "SymbolChannel {}:{} connecting to {}:{} {}", exchange_name_, symbol_,
           host, port, path);

  auto self = shared_from_this();
  ws_->Connect(host, port, path, [this, self](bool success) {
    if (success) {
      Subscribe();
      ws_->StartRead([this](const char* data, size_t size) { OnMessage(data, size); });
      LOG_INFO(GetLogger(), "Read loop started for {}:{}", exchange_name_, symbol_);
    } else {
      LOG_WARNING(GetLogger(), "{}:{} connect failed, retrying in 2s...", exchange_name_,
                  symbol_);
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::seconds(2));
      timer->async_wait([this, self, timer](boost::system::error_code) {
        if (!SignalHandler::IsShutdownRequested()) Start();
      });
    }
  });
}

void SymbolChannel::Stop() { if (ws_) ws_->Close(); }

void SymbolChannel::Subscribe() {
  if (exchange_name_ == "binance") {
    std::string name = symbol_;
    for (auto& c : name) c = static_cast<char>(std::tolower(c));
    std::string sub = R"({"method":"SUBSCRIBE","params":[")" + name + R"(@trade"],"id":1})";
    LOG_INFO(GetLogger(), "{}:{} sending subscribe: {}", exchange_name_, symbol_, sub);
    ws_->Write(sub);
  } else if (exchange_name_ == "gateio") {
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::string ticker_sub = R"({"time":)" + std::to_string(now_sec) +
                             R"(,"channel":"futures.tickers","event":"subscribe","payload":[")" +
                             symbol_ + R"("]})";
    LOG_INFO(GetLogger(), "{}:{} sending ticker subscribe: {}", exchange_name_, symbol_,
             ticker_sub);
    ws_->Write(ticker_sub);

    auto depth_sub = std::make_shared<std::string>(
        R"({"time":)" + std::to_string(now_sec) +
        R"(,"channel":"futures.order_book","event":"subscribe","payload":[")" +
        symbol_ + "\",\"" + std::to_string(depth_level_) + "\",\"0\"]}");

    auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::milliseconds(500));
    timer->async_wait([this, depth_sub, timer](boost::system::error_code) {
      LOG_INFO(GetLogger(), "{}:{} sending depth subscribe: {}", exchange_name_, symbol_,
               *depth_sub);
      ws_->Write(*depth_sub);
    });
  }
}

void SymbolChannel::OnMessage(const char* data, size_t size) {
  if (size == 0 || shard_queues_.empty()) return;

  auto json_data = std::make_shared<char[]>(size + kSimdjsonPadding);
  std::memcpy(json_data.get(), data, size);
  std::memset(json_data.get() + size, 0, kSimdjsonPadding);

  RawMessage msg{json_data, size, channel_id_, exchange_name_};

  uint32_t shard = (channel_id_ ^ std::hash<std::string>{}(exchange_name_)) % shard_queues_.size();
  shard_queues_[shard]->Push(std::move(msg));
}

}  // namespace sqc
