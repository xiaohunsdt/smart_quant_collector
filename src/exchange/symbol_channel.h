#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>

#include "shard_queue.h"

namespace sqc {

namespace net = boost::asio;

class WsClient;

class SymbolChannel : public std::enable_shared_from_this<SymbolChannel> {
 public:
  SymbolChannel(std::string exchange_name, std::string channel_type,
                std::string ws_url, std::string rest_host,
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
  const std::string& exchange_name() const { return exchange_name_; }
  const std::string& channel_type() const { return channel_type_; }
  uint32_t channel_id() const { return channel_id_; }

 private:
  void OnMessage(const char* data, size_t size);
  void Subscribe();

  std::string exchange_name_;
  std::string channel_type_;
  std::string ws_url_;
  std::string rest_host_;
  std::string symbol_;
  uint32_t depth_level_;
  uint32_t channel_id_;
  net::io_context& ioc_;
  net::ssl::context& ssl_ctx_;
  std::unique_ptr<WsClient> ws_;
  std::vector<std::shared_ptr<ShardQueue>> shard_queues_;

  // F17/F27: exponential backoff reconnect state
  uint32_t reconnect_attempts_ = 0;
  static constexpr uint32_t kMaxReconnectDelaySec = 60;
};

}  // namespace sqc
