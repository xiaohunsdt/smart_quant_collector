#include "storage/client/dolphindb_client.h"


#include <algorithm>

#include "common/logger_init.h"
#include "config/config_loader.h"
#include "storage/client/dolphindb_schema.h"
#include "quill/LogMacros.h"
#include "storage/timestamp_util.h"

namespace sqc {

namespace {

// Steady-clock ticks as a plain integer (nanoseconds since steady epoch).
// Used to back the atomic last_health_check_ns_ field without dragging the
// whole std::chrono::time_point through a shared-lock race.
std::chrono::steady_clock::rep NowSteadyNs() noexcept {
  return std::chrono::steady_clock::now().time_since_epoch().count();
}

}  // namespace

DolphinDBClient::DolphinDBClient() : conn_(std::make_unique<dolphindb::DBConnection>(false, false)) {}

DolphinDBClient::~DolphinDBClient() { Disconnect(); }

bool DolphinDBClient::InitSchema() {
  // Delegate the DDL to the DolphinDBSchema component. Build its config from
  // the live config (this runs only during Connect/Reconnect, serialized).
  const auto& dd = Config::Instance().storage.dolphindb;
  DolphinDBSchema::Config scfg;
  scfg.dfs_db_path = dd.dfs_db_path;
  scfg.hash_buckets = dd.hash_buckets;
  scfg.partition_granularity = dd.partition_granularity;
  scfg.mtw_batch_size = dd.mtw_batch_size;
  scfg.mtw_throttle_sec = dd.mtw_throttle_sec;
  return conn_ ? schema_.InitSchema(*conn_, scfg) : false;
}

bool DolphinDBClient::Connect(const std::string& host, uint16_t port, const std::string& user, const std::string& password) {
  host_ = host;
  port_ = port;
  user_ = user;
  password_ = SecureString::FromPlain(password);

  try {
    if(!conn_->connect(host, port, user, password)) {
      LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) returned false", host, port);
      return false;
    }

    conn_->login(user, password, false);
    last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);
    LOG_INFO(GetLogger(), "DolphinDB: connected to {}:{} as {}", host, port, user);

    // Auto-create schema if enabled
    if(Config::Instance().storage.dolphindb.auto_init_schema) {
      if(!InitSchema()) {
        LOG_ERROR(GetLogger(), "DolphinDB: InitSchema failed");
        conn_->close();
        // conn_.reset() is safe here: Connect() only runs during startup
        // (main thread, single-threaded) or under StorageRouter's
        // dolphindb_mtx_ exclusive lock (TryReconnect → Reconnect).
        conn_.reset();
        return false;
      }
    }

    if(!InitWriters()) {
      LOG_ERROR(GetLogger(), "DolphinDB: failed to init MTW writers");
      conn_->close();
      conn_.reset();
      return false;
    }
    connected_.store(true, std::memory_order_relaxed);
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) exception: {}", host, port, e.what());
    return false;
  }
}


// ============================================================
// InitWriters / DestroyWriters
// ============================================================

bool DolphinDBClient::InitWriters() {
  const auto& cfg = Config::Instance().storage.dolphindb;
  try {
    // password_.get() returns const std::string&; MTW copies it into internal pswd_ member.
    const auto& pw = password_.get();
    trades_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "trades_stream", false, false, nullptr,
                                                                           cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count,
                                                                           "trade_date", nullptr, dolphindb::MultithreadedTableWriter::M_Append);
    ob_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "orderbook_stream", false, false, nullptr,
                                                                       cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count, "trade_date",
                                                                       nullptr, dolphindb::MultithreadedTableWriter::M_Append);
    bt_writer_ = std::make_unique<dolphindb::MultithreadedTableWriter>(host_, port_, user_, pw, "", "bookticker_stream", false, false, nullptr,
                                                                       cfg.mtw_batch_size, cfg.mtw_throttle_sec, cfg.mtw_thread_count, "trade_date",
                                                                       nullptr, dolphindb::MultithreadedTableWriter::M_Append);
    return true;
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW init exception: {}", e.what());
    DestroyWriters();
    return false;
  }
}

void DolphinDBClient::DestroyWriters(bool skip_drain) {
  auto drain = [skip_drain](auto& writer, const char* name) {
    if(!writer) return;
    try {
      dolphindb::MultithreadedTableWriter::Status status;
      writer->getStatus(status);
      if(status.unsentRows > 0 || status.sentRows > 0) {
        LOG_INFO(GetLogger(), "DolphinDB: draining {} — sent={}, unsent={}, failed={}", name, status.sentRows, status.unsentRows,
                 status.sendFailedRows);
      }
      if(!skip_drain) {
        writer->waitForThreadCompletion();
      } else {
        LOG_WARNING(GetLogger(), "DolphinDB: skipping drain for {} (shutdown)", name);
      }
      // Read status again after drain
      dolphindb::MultithreadedTableWriter::Status finalStatus;
      writer->getStatus(finalStatus);
      if(finalStatus.unsentRows > 0 || finalStatus.sendFailedRows > 0) {
        LOG_WARNING(GetLogger(), "DolphinDB: {} drain incomplete — unsent={}, failed={}", name, finalStatus.unsentRows, finalStatus.sendFailedRows);
      }
    } catch(const std::exception& e) {
      LOG_WARNING(GetLogger(), "DolphinDB: drain {} exception: {}", name, e.what());
    }
    writer.reset();
  };

  drain(trades_writer_, "trades");
  drain(ob_writer_, "orderbook");
  drain(bt_writer_, "bookticker");
}

void DolphinDBClient::Disconnect() {
  DestroyWriters(/*skip_drain=*/true);
  if(!conn_) return;  // already disconnected — skip logging (safe during static destruction)
  try {
    conn_->close();
  } catch(const std::exception& e) {
    LOG_WARNING(GetLogger(), "DolphinDB: close() exception: {}", e.what());
  }
  conn_.reset();
  connected_.store(false, std::memory_order_relaxed);
  LOG_INFO(GetLogger(), "DolphinDB: disconnected");
}

// ============================================================
// IsHealthy — real ping with throttle
// ============================================================

bool DolphinDBClient::IsHealthy() {
  bool c = connected_.load(std::memory_order_relaxed);
  if(!c) return false;

  const auto& cfg = Config::Instance().storage.dolphindb;
  if(cfg.health_check_interval_ms == 0) return c;

  // Elapsed since last successful activity, computed from the atomic ticks
  // (steady-clock epoch is monotonic, so duration math is valid across threads).
  const auto now_ns = NowSteadyNs();
  const auto last_ns = last_health_check_ns_.load(std::memory_order_relaxed);
  // steady_clock ticks are nanoseconds on this platform (Linux); cast to ms.
  const auto elapsed_ms = static_cast<uint64_t>((now_ns - last_ns) / 1'000'000);
  if(elapsed_ms < cfg.health_check_interval_ms) {
    return c;  // Use cached result within interval
  }

  // Perform lightweight ping.
  // Caller (StorageRouter) holds dolphindb_mtx_, serializing with Disconnect/Reconnect.
  try {
    auto result = conn_->run("1+1");
    if(result->isNull() || result->getInt() != 2) {
      connected_.store(false, std::memory_order_relaxed);
      return false;
    }
    last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);
    return true;
  } catch(...) {
    connected_.store(false, std::memory_order_relaxed);
    return false;
  }
}

bool DolphinDBClient::Reconnect() {
  LOG_INFO(GetLogger(), "DolphinDB: attempting reconnect to {}:{}", host_, port_);

  // Step 1: Destroy old MTW writers.
  // Caller (StorageRouter::TryReconnect) holds dolphindb_mtx_ exclusive,
  // so hot-path threads are blocked — no risk of stale writer pointers.
  DestroyWriters();
  // Caller (StorageRouter::TryReconnect) holds dolphindb_mtx_ exclusive lock.
  try {
    if(conn_) conn_->close();
  } catch(...) {
  }
  connected_.store(false, std::memory_order_relaxed);
  conn_ = std::make_unique<dolphindb::DBConnection>(false, false);

  try {
    const auto& pw = password_.get();
    if(!conn_->connect(host_, port_, user_, pw)) {
      LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) returned false", host_, port_);
      return false;
    }
    conn_->login(user_, pw, false);
    last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);

    // Re-initialize schema (idempotent — handles server restart from scratch)
    if(Config::Instance().storage.dolphindb.auto_init_schema) {
      if(!InitSchema()) {
        LOG_WARNING(GetLogger(), "DolphinDB: InitSchema failed on reconnect");
        conn_->close();
        conn_.reset();
        return false;
      }
    }

    if(!InitWriters()) {
      LOG_WARNING(GetLogger(), "DolphinDB: failed to init MTW on reconnect");
      conn_->close();
      conn_.reset();
      return false;
    }

    connected_.store(true, std::memory_order_relaxed);
    LOG_INFO(GetLogger(), "DolphinDB: reconnected to {}:{}", host_, port_);
    return true;
  } catch(const std::exception& e) {
    LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) exception: {}", host_, port_, e.what());
    return false;
  }
}

// ============================================================
// RegisterChannel / UsecToDateInt / ValidateSchema
// ============================================================

void DolphinDBClient::RegisterChannel(uint32_t channel_id, std::string exchange, std::string market_type, std::string symbol, uint32_t depth_level) {
  registry_.Register(channel_id, std::move(exchange), std::move(market_type), std::move(symbol), depth_level);
}

bool DolphinDBClient::ValidateSchema() {
  if(!connected_.load(std::memory_order_relaxed) || !conn_) return false;
  const auto& cfg = Config::Instance().storage.dolphindb;
  return schema_.ValidateSchema(*conn_, cfg.dfs_db_path);
}


// ============================================================
// TableInsertTrades
// ============================================================

bool DolphinDBClient::TableInsertTrades(const std::vector<TickData>& batch) {
  if(!connected_.load(std::memory_order_relaxed) || !trades_writer_) return false;
  if(batch.empty()) return true;

  // Reusable fallback buffers for unregistered channels — the hot path passes
  // const references straight from ChannelMeta, avoiding 3 std::string heap
  // allocations per row that the previous copy-from-map code incurred.
  const std::string empty;
  dolphindb::ErrorCodeInfo errorInfo;
  for(const auto& t : batch) {
    const ChannelMeta* meta = ResolveChannel(t.channel_id);
    const std::string& exchange = meta ? meta->exchange : empty;
    const std::string& market_type = meta ? meta->market_type : empty;
    const std::string& symbol = meta ? meta->symbol : t.symbol;
    int trade_date = timestamp_util::UsecToDateInt(t.exchange_timestamp);
    if(!timestamp_util::IsValidTradeDate(trade_date)) {
      LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for trade symbol={} ts={}", trade_date, symbol, t.exchange_timestamp);
      return false;
    }
    int direction = t.is_buyer_maker ? -1 : 1;

    try {
      bool ok = trades_writer_->insert(errorInfo, static_cast<long long>(t.exchange_timestamp), static_cast<long long>(t.local_diff),
                                       static_cast<long long>(t.trade_id), t.price, t.quantity, direction, t.is_buyer_maker, symbol, exchange,
                                       market_type, trade_date);
      if(!ok) {
        LOG_ERROR(GetLogger(), "DolphinDB: MTW trades insert failed: {}", errorInfo.errorInfo);
        return false;
      }
    } catch(const std::exception& e) {
      LOG_ERROR(GetLogger(), "DolphinDB: MTW trades insert threw: {}", e.what());
      return false;
    }
  }
  last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertTrades queued {} rows", batch.size());
  return true;
}

// ============================================================
// TableInsertOrderbook — depth_level from ChannelMeta
// ============================================================

bool DolphinDBClient::TableInsertOrderbook(const DepthUpdateEvent& event, uint64_t local_ts) {
  if(!connected_.load(std::memory_order_relaxed) || !ob_writer_) return false;

  const std::string empty;
  uint32_t depth_level = kMaxOrderbookLevels;  // safe default

  const ChannelMeta* meta = ResolveChannel(event.channel_id);
  const std::string& exchange = meta ? meta->exchange : empty;
  const std::string& market_type = meta ? meta->market_type : empty;
  const std::string& symbol = meta ? meta->symbol : event.symbol;
  if(meta) {
    if(meta->depth_level > 0) {
      depth_level = std::min(meta->depth_level, static_cast<uint32_t>(kMaxOrderbookLevels));
    } else {
      LOG_WARNING(GetLogger(), "DolphinDB: depth_level=0 for channel {}, using default {}", event.channel_id, depth_level);
    }
  } else {
    LOG_WARNING(GetLogger(), "DolphinDB: unregistered channel {} in TableInsertOrderbook", event.channel_id);
  }
  int trade_date = timestamp_util::UsecToDateInt(event.exchange_timestamp);
  if(!timestamp_util::IsValidTradeDate(trade_date)) {
    LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for orderbook symbol={} ts={}", trade_date, symbol, event.exchange_timestamp);
    return false;
  }

  // Stack arrays — zero heap (max kMaxOrderbookLevels = 100 levels)
  double bid_prices_data[kMaxOrderbookLevels] = {};
  double bid_sizes_data[kMaxOrderbookLevels] = {};
  double ask_prices_data[kMaxOrderbookLevels] = {};
  double ask_sizes_data[kMaxOrderbookLevels] = {};
  for(uint32_t i = 0; i < depth_level; ++i) {
    bid_prices_data[i] = i < event.bid_count ? event.bids[i].price : 0.0;
    bid_sizes_data[i] = i < event.bid_count ? event.bids[i].quantity : 0.0;
    ask_prices_data[i] = i < event.ask_count ? event.asks[i].price : 0.0;
    ask_sizes_data[i] = i < event.ask_count ? event.asks[i].quantity : 0.0;
  }

  // Thread-local pre-allocated vectors (zero heap after first use per thread).
  // Each parser thread gets its own set — no shared state, no data race.
  thread_local dolphindb::VectorSP t_bid_prices;
  thread_local dolphindb::VectorSP t_bid_sizes;
  thread_local dolphindb::VectorSP t_ask_prices;
  thread_local dolphindb::VectorSP t_ask_sizes;
  if(t_bid_prices.isNull()) {
    t_bid_prices = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_bid_sizes = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_ask_prices = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
    t_ask_sizes = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, kMaxOrderbookLevels, true);
  }

  // clear() preserves capacity, appendDouble is allocation-free
  t_bid_prices->clear();
  t_bid_sizes->clear();
  t_ask_prices->clear();
  t_ask_sizes->clear();

  t_bid_prices->appendDouble(bid_prices_data, static_cast<int>(depth_level));
  t_bid_sizes->appendDouble(bid_sizes_data, static_cast<int>(depth_level));
  t_ask_prices->appendDouble(ask_prices_data, static_cast<int>(depth_level));
  t_ask_sizes->appendDouble(ask_sizes_data, static_cast<int>(depth_level));

  dolphindb::ErrorCodeInfo errorInfo;
  try {
    bool ok = ob_writer_->insert(errorInfo, static_cast<long long>(event.exchange_timestamp), static_cast<long long>(local_ts), symbol, exchange,
                                 market_type, trade_date, t_bid_prices, t_bid_sizes, t_ask_prices, t_ask_sizes);
    if(!ok) {
      LOG_ERROR(GetLogger(), "DolphinDB: MTW orderbook insert failed: {}", errorInfo.errorInfo);
      return false;
    }
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW orderbook insert threw: {}", e.what());
    return false;
  }
  last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertOrderbook queued");
  return true;
}

// ============================================================
// TableInsertBookTicker
// ============================================================

bool DolphinDBClient::TableInsertBookTicker(const BookTickerEvent& event) {
  if(!connected_.load(std::memory_order_relaxed) || !bt_writer_) return false;

  const std::string empty;
  const ChannelMeta* meta = ResolveChannel(event.channel_id);
  const std::string& exchange = meta ? meta->exchange : empty;
  const std::string& market_type = meta ? meta->market_type : empty;
  const std::string& symbol = meta ? meta->symbol : event.symbol;
  int trade_date = timestamp_util::UsecToDateInt(event.exchange_timestamp);
  if(!timestamp_util::IsValidTradeDate(trade_date)) {
    LOG_ERROR(GetLogger(), "DolphinDB: invalid trade_date {} for bookticker symbol={} ts={}", trade_date, symbol, event.exchange_timestamp);
    return false;
  }

  dolphindb::ErrorCodeInfo errorInfo;
  try {
    bool ok = bt_writer_->insert(errorInfo, static_cast<long long>(event.exchange_timestamp), static_cast<long long>(event.local_diff),
                                 event.best_bid_price, event.best_bid_qty, event.best_ask_price, event.best_ask_qty, symbol, exchange, market_type,
                                 trade_date);
    if(!ok) {
      LOG_ERROR(GetLogger(), "DolphinDB: MTW bookticker insert failed: {}", errorInfo.errorInfo);
      return false;
    }
  } catch(const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: MTW bookticker insert threw: {}", e.what());
    return false;
  }
  last_health_check_ns_.store(NowSteadyNs(), std::memory_order_relaxed);
  LOG_DEBUG(GetLogger(), "DolphinDB: TableInsertBookTicker queued");
  return true;
}

}  // namespace sqc
