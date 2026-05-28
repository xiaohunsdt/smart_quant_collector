#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>

#include "channel_spec.h"
#include "shard_queue.h"

namespace sqc {

namespace net = boost::asio;

class WsClient;
class RecvBuffer;

class ExchangeChannel : public std::enable_shared_from_this<ExchangeChannel> {
 public:
  ExchangeChannel(const ChannelSpec& spec, net::io_context& ioc,
                  net::ssl::context& ssl_ctx,
                  std::vector<std::shared_ptr<ShardQueue>> shard_queues,
                  const std::unordered_map<std::string, uint32_t>& symbol_to_channel_id);

  ~ExchangeChannel();
  ExchangeChannel(const ExchangeChannel&) = delete;
  ExchangeChannel& operator=(const ExchangeChannel&) = delete;

  void Start();
  void Stop();
  const ChannelSpec& spec() const { return spec_; }

 private:
  void OnMessage(const char* data, size_t size);
  void Subscribe();
  uint32_t ComputeShardIndex(std::string_view symbol) const;

  ChannelSpec spec_;
  net::io_context& ioc_;
  net::ssl::context& ssl_ctx_;
  std::unique_ptr<WsClient> ws_;
  std::unique_ptr<RecvBuffer> recv_buffer_;
  std::vector<std::shared_ptr<ShardQueue>> shard_queues_;
  const std::unordered_map<std::string, uint32_t>& symbol_to_channel_id_;
};

// Builds the Gate.io futures.order_book subscribe JSON payload.
// payload contains only contract names — depth_level and interval are
// client-side config, not API subscribe parameters.
std::string BuildGateioOrderBookSubscribePayload(
    const std::vector<SymbolSpec>& symbols,
    uint64_t timestamp_seconds);

}  // namespace sqc
