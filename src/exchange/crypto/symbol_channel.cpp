#include "symbol_channel.h"

#include <boost/asio/ssl.hpp>
#include <chrono>

#include "common/logger_init.h"
#include "quill/LogMacros.h"
#include "src/common/signal_handler.h"
#include "src/common/simdjson_utils.h"
#include "src/network/ws_client.h"

namespace sqc {
namespace {

struct ParsedUrl {
  std::string host;
  std::string port = "443";
  std::string path = "/";
};

ParsedUrl ParseWsUrl(std::string_view ws_url) {
  std::string url(ws_url);
  ParsedUrl result{};
  if(url.starts_with("wss://")) url = url.substr(6);
  auto colon = url.find(':');
  auto slash = url.find('/');
  if(colon != std::string::npos && (slash == std::string::npos || colon < slash)) {
    result.host = url.substr(0, colon);
    result.port = url.substr(colon + 1, slash - colon - 1);
  } else if(slash != std::string::npos) {
    result.host = url.substr(0, slash);
  } else {
    result.host = url;
  }
  if(slash != std::string::npos) result.path = url.substr(slash);
  return result;
}

}  // namespace

SymbolChannel::SymbolChannel(const ExchangeAdapter* adapter, std::string symbol, uint32_t depth_level, uint32_t channel_id, net::io_context& ioc,
                             net::ssl::context& ssl_ctx, std::vector<std::shared_ptr<ShardQueue>> shard_queues)
    : adapter_(adapter),
      symbol_(std::move(symbol)),
      depth_level_(depth_level),
      channel_id_(channel_id),
      ioc_(ioc),
      ssl_ctx_(ssl_ctx),
      shard_queues_(std::move(shard_queues)) {}

SymbolChannel::~SymbolChannel() = default;

void SymbolChannel::Start() {
  groups_ = adapter_->build_subscribes(symbol_, depth_level_);
  if(groups_.empty()) return;
  ws_clients_.resize(groups_.size());

  for(size_t g = 0; g < groups_.size(); ++g) {
    std::string_view ws_url = groups_[g].ws_url;

    auto parsed = ParseWsUrl(ws_url);

    LOG_INFO(GetLogger(), "SymbolChannel {}:{} group {} connecting to {}:{} {}", adapter_->name, symbol_, g, parsed.host, parsed.port, parsed.path);

    auto ws = std::make_unique<WsClient>(ioc_, ssl_ctx_);
    auto* ws_raw = ws.get();
    ws_clients_[g] = std::move(ws);

    // Apply vendor-specific handshake headers declared on the adapter (e.g.
    // Gate.io's X-Gate-* headers) instead of branching on the exchange name.
    for(const auto& h : adapter_->ws_headers) ws_raw->AddHeader(h.name, h.value);

    auto self = shared_from_this();
    ws_raw->Connect(parsed.host, parsed.port, parsed.path, [this, self, g](bool success) {
      if(success) {
        reconnect_attempts_ = 0;
        ws_clients_[g]->SetDisconnectHandler([this, self]() { OnDisconnect(); });
        SendGroupSubscriptions(g);
        ws_clients_[g]->StartRead([this](const char* data, size_t size) { OnMessage(data, size); });
        LOG_INFO(GetLogger(), "Read loop started for {}:{} group {}", adapter_->name, symbol_, g);
      } else {
        LOG_WARNING(GetLogger(), "{}:{} group {} connect failed", adapter_->name, symbol_, g);
        if(!is_reconnecting_) ScheduleReconnect();
      }
    });
  }
}

void SymbolChannel::Stop() {
  // Cancel pending delayed-subscription timers to prevent stale
  // callbacks from firing on new, unconnected WsClients after reconnect.
  for(auto& t : pending_timers_) t->cancel();
  pending_timers_.clear();
  for(auto& ws : ws_clients_) {
    if(ws) ws->Close();
  }
}

void SymbolChannel::OnDisconnect() {
  if(is_reconnecting_) return;
  LOG_WARNING(GetLogger(), "{}:{} disconnected, reconnecting...", adapter_->name, symbol_);
  if(SignalHandler::IsShutdownRequested()) return;

  // Cancel pending delayed-subscription timers before reconnecting.
  for(auto& t : pending_timers_) t->cancel();
  pending_timers_.clear();

  for(auto& ws : ws_clients_) {
    if(ws) ws->Close();
  }

  ScheduleReconnect();
}

void SymbolChannel::ScheduleReconnect() {
  // Exponential backoff capped at kMaxReconnectDelaySec, retried via a timer
  // that re-arms Start() unless shutdown was requested. The timer is held by
  // its own async_wait callback (self + timer capture) so it outlives this call.
  is_reconnecting_ = true;
  uint32_t delay = 1u << std::min(reconnect_attempts_, 6u);
  delay = std::min(delay, kMaxReconnectDelaySec);
  ++reconnect_attempts_;
  auto self = shared_from_this();
  auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::seconds(delay));
  timer->async_wait([this, self, timer](boost::system::error_code) {
    if(!SignalHandler::IsShutdownRequested()) {
      is_reconnecting_ = false;
      Start();
    }
  });
}

void SymbolChannel::SendGroupSubscriptions(size_t g) {
  if(g >= groups_.size()) return;
  const auto& group = groups_[g];
  for(size_t i = 0; i < group.messages.size(); ++i) {
    const auto& msg = group.messages[i];
    if(msg.delay_ms == 0) {
      LOG_INFO(GetLogger(), "{}:{} sending subscribe: {}", adapter_->name, symbol_, msg.payload);
      ws_clients_[g]->Write(msg.payload);
    } else {
      auto timer = std::make_shared<net::steady_timer>(ioc_, std::chrono::milliseconds(msg.delay_ms));
      pending_timers_.push_back(timer);
      auto self = shared_from_this();
      timer->async_wait([this, self, g, i, timer](boost::system::error_code ec) {
        if(ec || SignalHandler::IsShutdownRequested()) return;  // cancelled or shutting down
        LOG_INFO(GetLogger(), "{}:{} sending subscribe: {}", adapter_->name, symbol_, groups_[g].messages[i].payload);
        ws_clients_[g]->Write(groups_[g].messages[i].payload);
      });
    }
  }
}

void SymbolChannel::OnMessage(const char* data, size_t size) {
  if(size == 0 || shard_queues_.empty()) return;

  RawMessage msg;
  if(!msg.allocate(size)) return;
  std::memcpy(msg.buffer(), data, size);
  std::memset(msg.buffer() + size, 0, kSimdjsonPadding);
  msg.size = size;
  msg.channel_id = channel_id_;
  msg.recv_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  std::memcpy(msg.symbol, symbol_.c_str(), std::min(symbol_.size(), sizeof(msg.symbol) - 1));
  msg.parse_fn = adapter_->parse;

  simdjson::ondemand::document doc;
  if(simdjson_parser_.iterate(msg.buffer(), size, size + kSimdjsonPadding).get(doc) == simdjson::SUCCESS) {
    msg.event_type = adapter_->peek_event_type(doc);
  } else {
    // simdjson docs warn against reusing a parser after a failed iterate();
    // reset it so the next message gets a clean state.
    simdjson_parser_ = simdjson::ondemand::parser{};
  }

  if(msg.event_type == EventType::UNKNOWN) {
    LOG_WARNING(GetLogger(), "{}:{} unknown event type for message: {}", adapter_->name, symbol_, std::string_view(data, size));
    return;
  }

  uint32_t shard = (channel_id_ ^ std::hash<std::string_view>{}(adapter_->name)) % shard_queues_.size();
  shard_queues_[shard]->Push(std::move(msg));
}

}  // namespace sqc
