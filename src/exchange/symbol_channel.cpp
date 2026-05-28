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
      reconnect_attempts_ = 0;  // reset on successful connection

      // F17: auto-reconnect on disconnect with exponential backoff
      ws_->SetDisconnectHandler([this, self]() {
        LOG_WARNING(GetLogger(), "{}:{} disconnected, reconnecting...", exchange_name_,
                    symbol_);
        if (!SignalHandler::IsShutdownRequested()) {
          uint32_t delay = 1u << std::min(reconnect_attempts_, 6u);  // 1,2,4,8,16,32,64
          delay = std::min(delay, kMaxReconnectDelaySec);
          reconnect_attempts_++;
          auto timer = std::make_shared<net::steady_timer>(
              ioc_, std::chrono::seconds(delay));
          timer->async_wait([this, self, timer](boost::system::error_code) {
            if (!SignalHandler::IsShutdownRequested()) Start();
          });
        }
      });

      Subscribe();
      ws_->StartRead([this](const char* data, size_t size) { OnMessage(data, size); });
      LOG_INFO(GetLogger(), "Read loop started for {}:{}", exchange_name_, symbol_);
    } else {
      uint32_t delay = 1u << std::min(reconnect_attempts_, 6u);
      delay = std::min(delay, kMaxReconnectDelaySec);
      reconnect_attempts_++;
      LOG_WARNING(GetLogger(), "{}:{} connect failed, retrying in {}s...",
                  exchange_name_, symbol_, delay);
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::seconds(delay));
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

    std::string trade_sub = R"({"method":"SUBSCRIBE","params":[")" + name +
                            R"(@trade"],"id":1})";
    LOG_INFO(GetLogger(), "{}:{} sending trade subscribe: {}", exchange_name_, symbol_, trade_sub);
    ws_->Write(trade_sub);

    auto depth_sub = std::make_shared<std::string>(
        R"({"method":"SUBSCRIBE","params":[")" + name + R"(@depth@100ms"],"id":2})");
    auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::milliseconds(500));
    timer->async_wait([this, depth_sub, timer](boost::system::error_code) {
      LOG_INFO(GetLogger(), "{}:{} sending depth subscribe: {}", exchange_name_, symbol_,
               *depth_sub);
      ws_->Write(*depth_sub);
    });
  } else if (exchange_name_ == "gateio") {
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::string trade_sub = R"({"time":)" + std::to_string(now_sec) +
                             R"(,"channel":"futures.trades","event":"subscribe","payload":[")" +
                             symbol_ + R"("]})";
    LOG_INFO(GetLogger(), "{}:{} sending trades subscribe: {}", exchange_name_, symbol_,
             trade_sub);
    ws_->Write(trade_sub);

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

  // F09: zero-allocation for messages ≤ 16 KiB (covers ~99 % of frames)
  RawMessage msg;
  if (!msg.allocate(size)) return;
  std::memcpy(msg.buffer(), data, size);
  std::memset(msg.buffer() + size, 0, kSimdjsonPadding);
  msg.size = size;
  msg.channel_id = channel_id_;
  msg.recv_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
  std::strncpy(msg.exchange, exchange_name_.c_str(), sizeof(msg.exchange) - 1);
  msg.exchange[sizeof(msg.exchange) - 1] = '\0';

  uint32_t shard = (channel_id_ ^ std::hash<std::string>{}(exchange_name_)) % shard_queues_.size();
  shard_queues_[shard]->Push(std::move(msg));
}

}  // namespace sqc
