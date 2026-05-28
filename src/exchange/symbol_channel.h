#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>

#include "exchange/exchange_adapter.h"
#include "shard_queue.h"

namespace sqc {

namespace net = boost::asio;

class WsClient;

class SymbolChannel : public std::enable_shared_from_this<SymbolChannel> {
 public:
  SymbolChannel(const ExchangeAdapter* adapter, ChannelType channel_type,
                std::string symbol, uint32_t depth_level,
                uint32_t channel_id,
                net::io_context& ioc, net::ssl::context& ssl_ctx,
                std::vector<std::shared_ptr<ShardQueue>> shard_queues);

  ~SymbolChannel();
  SymbolChannel(const SymbolChannel&) = delete;
  SymbolChannel& operator=(const SymbolChannel&) = delete;

  void Start();
  void Stop();

  const std::string& symbol() const { return symbol_; }
  std::string_view exchange_name() const { return adapter_->name; }
  ChannelType channel_type() const { return channel_type_; }
  uint32_t channel_id() const { return channel_id_; }

 private:
  void OnMessage(const char* data, size_t size);
  void Subscribe();

  const ExchangeAdapter* adapter_;
  ChannelType channel_type_;
  std::string symbol_;
  uint32_t depth_level_;
  uint32_t channel_id_;
  net::io_context& ioc_;
  net::ssl::context& ssl_ctx_;
  std::unique_ptr<WsClient> ws_;
  std::vector<std::shared_ptr<ShardQueue>> shard_queues_;
  std::vector<std::pair<std::string, uint32_t>> subscribes_;

  uint32_t reconnect_attempts_ = 0;
  static constexpr uint32_t kMaxReconnectDelaySec = 60;
};

}  // namespace sqc
