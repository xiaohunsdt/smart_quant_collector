#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>

namespace sqc {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = net::ip::tcp;

class WsClient {
 public:
  using MessageHandler = std::function<void(const char* data, size_t size)>;

  WsClient(net::io_context& ioc, net::ssl::context& ssl_ctx);
  ~WsClient();

  WsClient(const WsClient&) = delete;
  WsClient& operator=(const WsClient&) = delete;

  using ConnectHandler = std::function<void(bool success)>;

  // Connect to host:port at the given WebSocket path
  void Connect(std::string_view host, std::string_view port, std::string_view path,
               ConnectHandler on_connect = {});

  // Start async read loop; handler called for each complete message
  void StartRead(MessageHandler handler);

  // Write a text message
  void Write(std::string_view message);

  // Synchronous close
  void Close();

  bool IsOpen() const { return is_open_; }

 private:
  void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
  void OnSslHandshake(beast::error_code ec);
  void OnHandshake(beast::error_code ec);

  net::io_context& ioc_;
  net::ssl::context& ssl_ctx_;
  tcp::resolver resolver_;
  websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
  beast::flat_buffer read_buffer_;

  std::string host_;
  std::string port_;
  std::string path_;
  bool is_open_ = false;
  ConnectHandler on_connect_;
};

}  // namespace sqc
