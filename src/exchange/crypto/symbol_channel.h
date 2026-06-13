#pragma once

#include <simdjson.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "exchange/exchange_adapter.h"
#include "exchange/shard_queue.h"

namespace sqc {

namespace net = boost::asio;

class WsClient;

class SymbolChannel : public std::enable_shared_from_this<SymbolChannel> {
 public:
  SymbolChannel(const ExchangeAdapter* adapter, std::string symbol, uint32_t depth_level, uint32_t channel_id, net::io_context& ioc,
                net::ssl::context& ssl_ctx, std::vector<std::shared_ptr<ShardQueue>> shard_queues);

  ~SymbolChannel();
  SymbolChannel(const SymbolChannel&) = delete;
  SymbolChannel& operator=(const SymbolChannel&) = delete;

  void Start();
  void Stop();

  const std::string& symbol() const { return symbol_; }
  std::string_view exchange_name() const { return adapter_->name; }
  ChannelType channel_type() const { return adapter_->channel_type; }
  uint32_t channel_id() const { return channel_id_; }

 private:
  void OnMessage(const char* data, size_t size);
  void SendGroupSubscriptions(size_t g);
  void OnDisconnect();

  const ExchangeAdapter* adapter_;
  std::string symbol_;
  uint32_t depth_level_;
  uint32_t channel_id_;
  net::io_context& ioc_;
  net::ssl::context& ssl_ctx_;
  std::vector<SubscriptionGroup> groups_;
  std::vector<std::unique_ptr<WsClient>> ws_clients_;
  std::vector<std::shared_ptr<ShardQueue>> shard_queues_;

  uint32_t reconnect_attempts_ = 0;
  bool is_reconnecting_ = false;
  static constexpr uint32_t kMaxReconnectDelaySec = 60;

  // Pending delayed-subscription timers — cancelled on disconnect/stop
  // to prevent stale timers from firing on new, unconnected WsClients.
  std::vector<std::shared_ptr<net::steady_timer>> pending_timers_;

  // Reused across messages to avoid heap allocation on the hot path.
  // On parse failure, the parser is reset (simdjson docs warn against
  // reusing a parser after a failed iterate()).
  simdjson::ondemand::parser simdjson_parser_;
};

}  // namespace sqc
