#include "http_utils.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

std::string HttpsGet(std::string_view host, std::string_view target) {
  try {
    net::io_context ioc;
    net::ssl::context ctx(net::ssl::context::tlsv12_client);
    ctx.set_verify_mode(net::ssl::verify_peer);
    ctx.set_default_verify_paths();

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    if (!SSL_set_tlsext_host_name(stream.native_handle(), std::string(host).c_str())) {
      beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
      LOG_ERROR(GetLogger(), "SSL_set_tlsext_host_name failed: {}", ec.message());
      return {};
    }

    auto const results = resolver.resolve(std::string(host), "443");
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(net::ssl::stream_base::client);

    http::request<http::string_body> req{http::verb::get, std::string(target), 11};
    req.set(http::field::host, std::string(host));
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.shutdown(ec);

    if (res.result() != http::status::ok) {
      LOG_ERROR(GetLogger(), "HTTP {} for {}: {}", static_cast<int>(res.result()),
                target, res.body());
      return {};
    }
    return res.body();
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "HttpsGet exception for {}: {}", target, e.what());
    return {};
  }
}

}  // namespace sqc
