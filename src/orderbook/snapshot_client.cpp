#include "snapshot_client.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include "simdjson.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"
#include "common/string_utils.h"

namespace sqc {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// ------------------------------------------------------------------
// Generic synchronous HTTPS GET helper
// ------------------------------------------------------------------
static std::string HttpsGet(std::string_view host, std::string_view target) {
  try {
    net::io_context ioc;
    net::ssl::context ctx(net::ssl::context::tlsv12_client);
    ctx.set_verify_mode(net::ssl::verify_peer);
    ctx.set_default_verify_paths();

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Set SNI hostname
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
    // Ignore eof/short_read errors on shutdown

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

// ------------------------------------------------------------------
// Parse helpers using simdjson
// ------------------------------------------------------------------
static bool ParseBinanceDepth(const std::string& json, OrderbookSnapshot& out) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto err = parser.parse(json).get(doc);
  if (err) {
    LOG_ERROR(GetLogger(), "Binance depth parse error: {}", simdjson::error_message(err));
    return false;
  }

  uint64_t last_id = 0;
  if (doc["lastUpdateId"].get_uint64().get(last_id) != simdjson::SUCCESS) {
    LOG_ERROR(GetLogger(), "Binance depth missing lastUpdateId");
    return false;
  }
  out.lastUpdateId = last_id;

  simdjson::dom::array bids;
  if (doc["bids"].get_array().get(bids) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : bids) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.bids[i].price = SvToDouble(psv);
      out.bids[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.bid_count = i;
  }

  simdjson::dom::array asks;
  if (doc["asks"].get_array().get(asks) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : asks) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.asks[i].price = SvToDouble(psv);
      out.asks[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.ask_count = i;
  }

  return true;
}

static bool ParseGateioDepth(const std::string& json, OrderbookSnapshot& out) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto err = parser.parse(json).get(doc);
  if (err) {
    LOG_ERROR(GetLogger(), "Gate.io depth parse error: {}", simdjson::error_message(err));
    return false;
  }

  // Gate.io returns "id" or "current" as the last update id
  uint64_t last_id = 0;
  if (doc["id"].get_uint64().get(last_id) != simdjson::SUCCESS) {
    (void)doc["current"].get_uint64().get(last_id);
  }
  out.lastUpdateId = last_id;

  simdjson::dom::array bids;
  if (doc["bids"].get_array().get(bids) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : bids) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.bids[i].price = SvToDouble(psv);
      out.bids[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.bid_count = i;
  }

  simdjson::dom::array asks;
  if (doc["asks"].get_array().get(asks) == simdjson::SUCCESS) {
    uint32_t i = 0;
    for (auto elem : asks) {
      if (i >= kMaxOrderbookLevels) break;
      simdjson::dom::array level;
      if (elem.get_array().get(level) != simdjson::SUCCESS) continue;
      auto price_it = level.begin();
      auto qty_it = level.begin();
      ++qty_it;
      if (price_it == level.end() || qty_it == level.end()) continue;
      std::string_view psv, qsv;
      if ((*price_it).get_string().get(psv) != simdjson::SUCCESS) continue;
      if ((*qty_it).get_string().get(qsv) != simdjson::SUCCESS) continue;
      out.asks[i].price = SvToDouble(psv);
      out.asks[i].quantity = SvToDouble(qsv);
      ++i;
    }
    out.ask_count = i;
  }

  return true;
}

// ------------------------------------------------------------------
// Public API
// ------------------------------------------------------------------
OrderbookSnapshot SnapshotClient::FetchSnapshot(std::string_view rest_host,
                                                std::string_view channel_type,
                                                std::string_view symbol,
                                                uint32_t limit) {
  if (rest_host.find("gateio") != std::string_view::npos)
    return FetchGateioSnapshot(rest_host, channel_type, symbol, limit);
  return FetchBinanceSnapshot(rest_host, symbol, limit);
}

OrderbookSnapshot SnapshotClient::FetchBinanceSnapshot(std::string_view rest_host,
                                                       std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};

  bool is_futures = (rest_host.find("fapi") != std::string_view::npos);
  const char* api_path = is_futures ? "/fapi/v1/depth" : "/api/v3/depth";

  std::string target = api_path;
  target += "?symbol=";
  target += symbol;
  target += "&limit=";
  target += std::to_string(limit);

  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Binance depth from {} for {} limit={}",
           rest_host, symbol, limit);

  std::string body = HttpsGet(rest_host, target);
  if (body.empty()) {
    LOG_ERROR(GetLogger(), "SnapshotClient: Binance HTTP empty response");
    return snapshot;
  }

  if (!ParseBinanceDepth(body, snapshot)) {
    LOG_ERROR(GetLogger(), "SnapshotClient: Binance depth parse failed");
  }
  return snapshot;
}

OrderbookSnapshot SnapshotClient::FetchGateioSnapshot(std::string_view rest_host,
                                                       std::string_view channel_type,
                                                       std::string_view symbol,
                                                       uint32_t limit) {
  OrderbookSnapshot snapshot{};

  bool is_spot = (channel_type == "spot");
  const char* api_path = is_spot ? "/api/v4/spot/order_book"
                                 : "/api/v4/futures/usdt/order_book";

  std::string target = api_path;
  target += "?";
  target += is_spot ? "currency_pair=" : "contract=";
  target += symbol;
  target += "&limit=";
  target += std::to_string(limit);
  if (is_spot) target += "&with_id=true";

  LOG_INFO(GetLogger(),
           "SnapshotClient: fetching Gate.io {}/order_book for {} limit={}",
           is_spot ? "spot" : "futures/usdt", symbol, limit);

  std::string body = HttpsGet(std::string(rest_host), target);
  if (body.empty()) {
    LOG_ERROR(GetLogger(), "SnapshotClient: Gate.io HTTP empty response");
    return snapshot;
  }

  if (!ParseGateioDepth(body, snapshot)) {
    LOG_ERROR(GetLogger(), "SnapshotClient: Gate.io depth parse failed");
  }
  return snapshot;
}

}  // namespace sqc
