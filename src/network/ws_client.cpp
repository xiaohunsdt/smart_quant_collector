#include "ws_client.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/post.hpp>

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

WsClient::WsClient(net::io_context& ioc, net::ssl::context& ssl_ctx) : ioc_(ioc), ssl_ctx_(ssl_ctx), resolver_(ioc), ws_(ioc_, ssl_ctx_) {}

WsClient::~WsClient() {
  // Safety-net synchronous close.
  // PRECONDITION: the io_context must be stopped and its thread joined
  // before this destructor runs (see main.cpp shutdown sequence).
  // After io_context::run() returns, no async handlers are touching ws_,
  // so a synchronous close is safe.
  if(!ioc_.stopped()) {
    LOG_ERROR(GetLogger(), "WsClient destroyed while io_context still running — data race risk");
  }
  assert(ioc_.stopped() && "Precondition: io_context must be stopped before WsClient destructor");
  if(is_open_.load(std::memory_order_acquire)) {
    is_open_.store(false, std::memory_order_release);
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
  }
}

void WsClient::AddHeader(std::string name, std::string value) { extra_headers_.emplace_back(std::move(name), std::move(value)); }

void WsClient::Connect(std::string_view host, std::string_view port, std::string_view path, ConnectHandler on_connect) {
  host_ = host;
  port_ = port;
  path_ = path;
  on_connect_ = std::move(on_connect);
  on_disconnect_ = {};

  resolver_.async_resolve(host_, port_, [this](beast::error_code ec, tcp::resolver::results_type results) { OnResolve(ec, results); });
}

void WsClient::OnResolve(beast::error_code ec, tcp::resolver::results_type results) {
  if(ec) {
    LOG_ERROR(GetLogger(), "WS resolve failed: {}", ec.message());
    if(on_connect_) on_connect_(false);
    return;
  }

  beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

  beast::get_lowest_layer(ws_).async_connect(
      results, [this](beast::error_code connect_ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep) {
        if(connect_ec) {
          LOG_ERROR(GetLogger(), "WS TCP connect failed: {}", connect_ec.message());
          if(on_connect_) on_connect_(false);
          return;
        }

        // F21: disable Nagle's algorithm for low-latency market data
        beast::get_lowest_layer(ws_).socket().set_option(net::ip::tcp::no_delay(true));

        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(websocket::stream_base::decorator([this](websocket::request_type& req) {
          req.set(beast::http::field::user_agent, "smart_quant_collector");
          for(const auto& [name, value] : extra_headers_) {
            req.set(name, value);
          }
        }));

        ws_.next_layer().async_handshake(net::ssl::stream_base::client, [this](beast::error_code ssl_ec) { OnSslHandshake(ssl_ec); });
      });
}

void WsClient::OnSslHandshake(beast::error_code ec) {
  if(ec) {
    LOG_ERROR(GetLogger(), "SSL handshake failed: {}", ec.message());
    if(on_connect_) on_connect_(false);
    return;
  }
  ws_.async_handshake(host_, path_, [this](beast::error_code hs_ec) { OnHandshake(hs_ec); });
}

void WsClient::OnHandshake(beast::error_code ec) {
  if(ec) {
    LOG_ERROR(GetLogger(), "WS handshake failed: {}", ec.message());
    if(on_connect_) on_connect_(false);
    return;
  }
  is_open_.store(true, std::memory_order_release);
  LOG_INFO(GetLogger(), "WS connected to {}:{}{}", host_, port_, path_);
  if(on_connect_) on_connect_(true);
}

void WsClient::SetDisconnectHandler(DisconnectHandler handler) { on_disconnect_ = std::move(handler); }

void WsClient::StartRead(MessageHandler handler) {
  ws_.async_read(read_buffer_, [this, handler](beast::error_code ec, size_t bytes_transferred) {
    if(ec) {
      LOG_ERROR(GetLogger(), "WS read failed: {}", ec.message());
      is_open_.store(false, std::memory_order_release);
      if(on_disconnect_) on_disconnect_();
      return;
    }
    auto* data = static_cast<const char*>(read_buffer_.data().data());
    handler(data, bytes_transferred);
    read_buffer_.consume(bytes_transferred);
    // Recurse: start next read
    if(is_open_.load(std::memory_order_acquire)) {
      StartRead(handler);
    }
  });
}

void WsClient::Write(std::string_view message) {
  // Dispatch to io_context thread to serialize with async_read.
  // Subscription messages are sent once at startup — not on the hot path.
  // Use shared_ptr<string> so the buffer outlives both the outer lambda
  // and the async_write completion handler.
  auto msg = std::make_shared<std::string>(message);
  net::post(ioc_, [this, msg]() {
    ws_.async_write(net::buffer(*msg), [msg](beast::error_code ec, size_t) {
      if(ec) LOG_ERROR(GetLogger(), "WS async_write failed: {}", ec.message());
    });
  });
}

void WsClient::Close() {
  // CAS ensures only one caller posts the close handler — prevents
  // double-close when Stop() and OnDisconnect() race.
  bool expected = true;
  if(!is_open_.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) return;
  net::post(ioc_, [this]() {
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
  });
}

}  // namespace sqc
