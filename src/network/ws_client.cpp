#include "ws_client.h"

#include <boost/asio/connect.hpp>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

WsClient::WsClient(net::io_context& ioc, net::ssl::context& ssl_ctx)
    : ioc_(ioc), ssl_ctx_(ssl_ctx), resolver_(ioc), ws_(ioc_, ssl_ctx_) {}

WsClient::~WsClient() {
  Close();
}

void WsClient::Connect(std::string_view host, std::string_view port,
                       std::string_view path,
                       ConnectHandler on_connect) {
  host_ = host;
  port_ = port;
  path_ = path;
  on_connect_ = std::move(on_connect);

  resolver_.async_resolve(
      host_, port_,
      [this](beast::error_code ec, tcp::resolver::results_type results) {
        OnResolve(ec, results);
      });
}

void WsClient::OnResolve(beast::error_code ec,
                         tcp::resolver::results_type results) {
  if (ec) {
    LOG_ERROR(GetLogger(), "WS resolve failed: {}", ec.message());
    if (on_connect_) on_connect_(false);
    return;
  }

  beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

  beast::get_lowest_layer(ws_).async_connect(
      results,
      [this](beast::error_code connect_ec,
             tcp::resolver::results_type::endpoint_type /*ep*/) {
        if (connect_ec) {
          LOG_ERROR(GetLogger(), "WS TCP connect failed: {}",
                    connect_ec.message());
          return;
        }

        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(
            websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
              req.set(beast::http::field::user_agent, "smart_quant_collector");
            }));

        ws_.next_layer().async_handshake(
            net::ssl::stream_base::client,
            [this](beast::error_code ssl_ec) { OnSslHandshake(ssl_ec); });
      });
}

void WsClient::OnSslHandshake(beast::error_code ec) {
  if (ec) {
    LOG_ERROR(GetLogger(), "SSL handshake failed: {}", ec.message());
    return;
  }
  ws_.async_handshake(host_, path_,
                      [this](beast::error_code hs_ec) { OnHandshake(hs_ec); });
}

void WsClient::OnHandshake(beast::error_code ec) {
  if (ec) {
    LOG_ERROR(GetLogger(), "WS handshake failed: {}", ec.message());
    if (on_connect_) on_connect_(false);
    return;
  }
  is_open_ = true;
  LOG_INFO(GetLogger(), "WS connected to {}:{}{}", host_, port_, path_);
  if (on_connect_) on_connect_(true);
}

void WsClient::StartRead(MessageHandler handler) {
  ws_.async_read(
      read_buffer_,
      [this, handler](beast::error_code ec, size_t bytes_transferred) {
        if (ec) {
          LOG_ERROR(GetLogger(), "WS read failed: {}", ec.message());
          is_open_ = false;
          return;
        }
        auto* data = static_cast<const char*>(read_buffer_.data().data());
        handler(data, bytes_transferred);
        read_buffer_.consume(bytes_transferred);
        // Recurse: start next read
        if (is_open_) {
          StartRead(handler);
        }
      });
}

void WsClient::Write(std::string_view message) {
  // Copy into shared_ptr so buffer outlives the async_write even if
  // another Write() is called before this one completes.
  auto buf = std::make_shared<std::string>(message.data(), message.size());
  ws_.async_write(net::buffer(*buf),[buf](beast::error_code ec, size_t /*bytes*/) -> void { 
    if (ec) LOG_ERROR(GetLogger(), "WS write failed: {}",  ec.message()); 
  });
}

void WsClient::Close() {
  if (!is_open_) return;
  beast::error_code ec;
  ws_.close(websocket::close_code::normal, ec);
  is_open_ = false;
  if (ec) {
    LOG_WARNING(GetLogger(), "WS close warning: {}", ec.message());
  }
}

}  // namespace sqc
