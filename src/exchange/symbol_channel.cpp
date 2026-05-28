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
    const ExchangeAdapter* adapter, std::string channel_type,
    std::string symbol, uint32_t depth_level,
    uint32_t channel_id,
    net::io_context& ioc, net::ssl::context& ssl_ctx,
    std::vector<std::shared_ptr<ShardQueue>> shard_queues)
    : adapter_(adapter),
      channel_type_(std::move(channel_type)),
      symbol_(std::move(symbol)),
      depth_level_(depth_level),
      channel_id_(channel_id),
      ioc_(ioc), ssl_ctx_(ssl_ctx),
      shard_queues_(std::move(shard_queues)) {
  ws_ = std::make_unique<WsClient>(ioc_, ssl_ctx_);
}

SymbolChannel::~SymbolChannel() = default;

void SymbolChannel::Start() {
  std::string_view ws_url = adapter_->endpoints(channel_type_).ws_url;
  std::string url(ws_url);
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

  LOG_INFO(GetLogger(), "SymbolChannel {}:{} connecting to {}:{} {}", adapter_->name, symbol_,
           host, port, path);

  auto self = shared_from_this();
  ws_->Connect(host, port, path, [this, self](bool success) {
    if (success) {
      reconnect_attempts_ = 0;

      ws_->SetDisconnectHandler([this, self]() {
        LOG_WARNING(GetLogger(), "{}:{} disconnected, reconnecting...", adapter_->name,
                    symbol_);
        if (!SignalHandler::IsShutdownRequested()) {
          uint32_t delay = 1u << std::min(reconnect_attempts_, 6u);
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
      LOG_INFO(GetLogger(), "Read loop started for {}:{}", adapter_->name, symbol_);
    } else {
      uint32_t delay = 1u << std::min(reconnect_attempts_, 6u);
      delay = std::min(delay, kMaxReconnectDelaySec);
      reconnect_attempts_++;
      LOG_WARNING(GetLogger(), "{}:{} connect failed, retrying in {}s...",
                  adapter_->name, symbol_, delay);
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::seconds(delay));
      timer->async_wait([this, self, timer](boost::system::error_code) {
        if (!SignalHandler::IsShutdownRequested()) Start();
      });
    }
  });
}

void SymbolChannel::Stop() { if (ws_) ws_->Close(); }

void SymbolChannel::Subscribe() {
  subscribes_ = adapter_->build_subscribes(channel_type_, symbol_, depth_level_);
  for (size_t i = 0; i < subscribes_.size(); ++i) {
    const auto& [msg, delay_ms] = subscribes_[i];
    if (delay_ms == 0) {
      LOG_INFO(GetLogger(), "{}:{} sending subscribe: {}", adapter_->name, symbol_, msg);
      ws_->Write(msg);
    } else {
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::milliseconds(delay_ms));
      timer->async_wait([this, i, timer](boost::system::error_code) {
        if (!SignalHandler::IsShutdownRequested()) {
          const auto& [m, _] = subscribes_[i];
          LOG_INFO(GetLogger(), "{}:{} sending subscribe: {}", adapter_->name, symbol_, m);
          ws_->Write(m);
        }
      });
    }
  }
}

void SymbolChannel::OnMessage(const char* data, size_t size) {
  if (size == 0 || shard_queues_.empty()) return;

  RawMessage msg;
  if (!msg.allocate(size)) return;
  std::memcpy(msg.buffer(), data, size);
  std::memset(msg.buffer() + size, 0, kSimdjsonPadding);
  msg.size = size;
  msg.channel_id = channel_id_;
  msg.recv_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
  msg.parse_fn = adapter_->parse;

  uint32_t shard = (channel_id_ ^ std::hash<std::string_view>{}(adapter_->name)) % shard_queues_.size();
  shard_queues_[shard]->Push(std::move(msg));
}

}  // namespace sqc
