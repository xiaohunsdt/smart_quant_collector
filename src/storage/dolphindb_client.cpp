#include "dolphindb_client.h"

#include <DolphinDB.h>

#include <sstream>

#include "common/logger_init.h"
#include "quill/LogMacros.h"

namespace sqc {

namespace {

/// Format a double value for SQL literal with full precision.
std::string FmtDouble(double v) {
  char buf[32];
  int n = std::snprintf(buf, sizeof(buf), "%.17g", v);
  return {buf, static_cast<size_t>(n)};
}

/// Extract null-terminated symbol string, escaping single quotes.
std::string EscapeSymbol(const char* sym, size_t max_len) {
  std::string result;
  result.reserve(max_len + 2);
  for (size_t i = 0; i < max_len && sym[i] != '\0'; ++i) {
    char c = sym[i];
    if (c == '\'') result += "''";
    else result += c;
  }
  return result;
}

}  // namespace

DolphinDBClient::DolphinDBClient() : conn_(std::make_unique<dolphindb::DBConnection>(false, false)) {}

DolphinDBClient::~DolphinDBClient() {
  Disconnect();
}

bool DolphinDBClient::Connect(const std::string& host, uint16_t port, const std::string& user, const std::string& password) {
  host_ = host;
  port_ = port;
  user_ = user;
  password_ = password;

  try {
    if (!conn_->connect(host, port, user, password)) {
      LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) returned false", host, port);
      return false;
    }
    conn_->login(user, password, false);
    connected_ = true;
    last_health_check_ = std::chrono::steady_clock::now();
    LOG_INFO(GetLogger(), "DolphinDB: connected to {}:{} as {}", host, port, user);
    return true;
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: connect({}:{}) exception: {}", host, port, e.what());
    return false;
  }
}

void DolphinDBClient::Disconnect() {
  if (conn_) {
    try {
      conn_->close();
    } catch (const std::exception& e) {
      LOG_WARNING(GetLogger(), "DolphinDB: close() exception: {}", e.what());
    }
  }
  connected_ = false;
  LOG_INFO(GetLogger(), "DolphinDB: disconnected");
}

bool DolphinDBClient::IsHealthy() const {
  return connected_ && conn_ != nullptr;
}

bool DolphinDBClient::Reconnect() {
  LOG_INFO(GetLogger(), "DolphinDB: attempting reconnect to {}:{}", host_, port_);

  // Tear down old connection and create a fresh one.
  try {
    if (conn_) conn_->close();
  } catch (...) {
    // Ignore close errors during reconnect.
  }
  connected_ = false;
  conn_ = std::make_unique<dolphindb::DBConnection>(false, false);

  try {
    if (!conn_->connect(host_, port_, user_, password_)) {
      LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) returned false", host_, port_);
      return false;
    }
    conn_->login(user_, password_, false);
    connected_ = true;
    last_health_check_ = std::chrono::steady_clock::now();
    LOG_INFO(GetLogger(), "DolphinDB: reconnected to {}:{}", host_, port_);
    return true;
  } catch (const std::exception& e) {
    LOG_WARNING(GetLogger(), "DolphinDB: reconnect({}:{}) exception: {}", host_, port_, e.what());
    return false;
  }
}

std::string DolphinDBClient::BuildInsertValues(const std::string& table_name, const std::vector<TickData>& batch) {
  // Columns: exchange_timestamp, local_diff, trade_id, price, quantity,
  //           channel_id, symbol, is_buyer_maker
  std::ostringstream sql;
  sql << "INSERT INTO " << table_name
      << " (exchange_timestamp,local_diff,trade_id,price,quantity,"
         "channel_id,symbol,is_buyer_maker) VALUES ";
  for (size_t i = 0; i < batch.size(); ++i) {
    if (i > 0) sql << ',';
    const auto& t = batch[i];
    sql << '(' << t.exchange_timestamp << ',' << t.local_diff << ','
        << t.trade_id << ',' << FmtDouble(t.price) << ','
        << FmtDouble(t.quantity) << ',' << t.channel_id << ",'"
        << EscapeSymbol(t.symbol, sizeof(t.symbol)) << "',"
        << (t.is_buyer_maker ? "true" : "false") << ')';
  }
  return sql.str();
}

std::string DolphinDBClient::BuildUpsertCall(const std::string& table_name, const std::vector<TickData>& batch) {
  // upsert!(table, newData, keyCols)
  // Builds: upsert!(table_name, table(...batch data...,
  //            [`col_names`]), [`key_cols`])
  std::ostringstream sql;
  sql << "upsert!(" << table_name << ", table(";

  // Column-order: exchange_timestamp, local_diff, trade_id, price, quantity,
  //               channel_id, symbol, is_buyer_maker
  for (size_t i = 0; i < batch.size(); ++i) {
    if (i > 0) sql << ',';
    const auto& t = batch[i];
    sql << t.exchange_timestamp << ' ' << t.local_diff << ' ' << t.trade_id
        << ' ' << FmtDouble(t.price) << ' ' << FmtDouble(t.quantity) << ' '
        << t.channel_id << ' '
        << EscapeSymbol(t.symbol, sizeof(t.symbol)) << ' '
        << (t.is_buyer_maker ? "true" : "false");
  }
  sql << ",[`exchange_timestamp,`local_diff,`trade_id,`price,`quantity,"
         "`channel_id,`symbol,`is_buyer_maker],"
         "[`channel_id,`exchange_timestamp,`trade_id])";
  return sql.str();
}

bool DolphinDBClient::TableInsert(const std::string& table_name, const std::vector<TickData>& batch) {
  if (!connected_) return false;
  if (batch.empty()) return true;

  try {
    std::string sql = BuildInsertValues(table_name, batch);
    conn_->run(sql);
    last_health_check_ = std::chrono::steady_clock::now();
    LOG_DEBUG(GetLogger(), "DolphinDB: tableInsert {} ({} rows)", table_name, batch.size());
    return true;
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: tableInsert({}) failed: {}", table_name, e.what());
    connected_ = false;
    return false;
  }
}

bool DolphinDBClient::Upsert(const std::string& table_name, const std::vector<TickData>& batch) {
  if (!connected_) return false;
  if (batch.empty()) return true;

  try {
    std::string sql = BuildUpsertCall(table_name, batch);
    conn_->run(sql);
    last_health_check_ = std::chrono::steady_clock::now();
    LOG_DEBUG(GetLogger(), "DolphinDB: upsert {} ({} rows)", table_name, batch.size());
    return true;
  } catch (const std::exception& e) {
    LOG_ERROR(GetLogger(), "DolphinDB: upsert({}) failed: {}", table_name, e.what());
    connected_ = false;
    return false;
  }
}

}  // namespace sqc
