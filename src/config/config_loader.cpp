#include "config_loader.h"

#include <fmt/format.h>

#include <cstdlib>
#include <stdexcept>

#include "src/orderbook/orderbook_event.h"
#include "yaml-cpp/yaml.h"

namespace sqc {

namespace {

SymbolConfig ParseSymbol(const YAML::Node& node) {
  SymbolConfig s;
  s.name = node["name"].as<std::string>();
  if(s.name.size() > 11) {
    throw std::runtime_error(fmt::format(
        "Symbol '{}' is {} chars (max 11). sizeof(TickData)==64 (1 cache line) requires symbol <= 11 chars.", s.name,
        s.name.size()));
  }
  if(node["enabled"]) s.enabled = node["enabled"].as<bool>();
  if(node["depth_level"]) {
    s.depth_level = node["depth_level"].as<uint32_t>();
    if(s.depth_level > kMaxOrderbookLevels) {
      throw std::runtime_error(
          fmt::format("Symbol '{}': depth_level {} exceeds kMaxOrderbookLevels ({})", s.name, s.depth_level, kMaxOrderbookLevels));
    }
  }
  if(node["record_tick"]) s.record_tick = node["record_tick"].as<bool>();
  if(node["persist_to_disk"]) s.persist_to_disk = node["persist_to_disk"].as<bool>();
  return s;
}

ChannelConfig ParseChannel(const YAML::Node& node) {
  ChannelConfig c;
  c.type = node["type"].as<std::string>();
  if(node["symbols"] && node["symbols"].IsSequence()) {
    for(const auto& sym : node["symbols"]) {
      c.symbols.push_back(ParseSymbol(sym));
    }
  }
  return c;
}

RithmicConfig ParseRithmicConfig(const YAML::Node& node) {
  RithmicConfig r;
  if (!node) return r;

  if (node["user"]) r.user = node["user"].as<std::string>();
  if (node["password"]) r.password = SecureString::FromPlain(node["password"].as<std::string>());
  if (node["shm_name"]) r.shm_name = node["shm_name"].as<std::string>();
  if (node["engine_core"]) r.engine_core = node["engine_core"].as<uint32_t>();
  if (node["forwarder_core"]) r.forwarder_core = node["forwarder_core"].as<uint32_t>();
  if (node["ssl_cert_file"]) r.ssl_cert_file = node["ssl_cert_file"].as<std::string>();
  if (node["domain_servers"]) r.domain_servers = node["domain_servers"].as<std::string>();
  if (node["domain_name"]) r.domain_name = node["domain_name"].as<std::string>();
  if (node["license_servers"]) r.license_servers = node["license_servers"].as<std::string>();
  if (node["local_broker"]) r.local_broker = node["local_broker"].as<std::string>();
  if (node["logger_servers"]) r.logger_servers = node["logger_servers"].as<std::string>();

  if (node["app_name"]) r.app_name = node["app_name"].as<std::string>();
  if (node["app_version"]) r.app_version = node["app_version"].as<std::string>();

  if (node["repository_connect_pt"]) r.repository_connect_pt = node["repository_connect_pt"].as<std::string>();
  if (node["md_connect_pt"]) r.md_connect_pt = node["md_connect_pt"].as<std::string>();
  if (node["md_connect_pt_agg"]) r.md_connect_pt_agg = node["md_connect_pt_agg"].as<std::string>();
  if (node["use_aggregated_md"]) r.use_aggregated_md = node["use_aggregated_md"].as<bool>();
  if (node["ih_connect_pt"]) r.ih_connect_pt = node["ih_connect_pt"].as<std::string>();
  if (node["ts_connect_pt"]) r.ts_connect_pt = node["ts_connect_pt"].as<std::string>();
  if (node["pnl_connect_pt"]) r.pnl_connect_pt = node["pnl_connect_pt"].as<std::string>();

  return r;
}

ExchangeConfig ParseExchange(const YAML::Node& node) {
  ExchangeConfig e;
  e.name = node["name"].as<std::string>();
  if(node["enabled"]) e.enabled = node["enabled"].as<bool>();
  if(node["channels"] && node["channels"].IsSequence()) {
    for(const auto& ch : node["channels"]) {
      e.channels.push_back(ParseChannel(ch));
    }
  }
  return e;
}

}  // namespace

RootConfig LoadConfig(const std::string& path) {
  YAML::Node root;
  try {
    root = YAML::LoadFile(path);
  } catch(const YAML::BadFile& e) {
    fmt::print(stderr, "FATAL: Config file not found: {}\n", path);
    std::exit(EXIT_FAILURE);
  } catch(const YAML::ParserException& e) {
    fmt::print(stderr, "FATAL: YAML parse error in {}: {}\n", path, e.what());
    std::exit(EXIT_FAILURE);
  }
  RootConfig config;

  if(root["global"]) {
    auto g = root["global"];
    if(g["environment"]) config.global.environment = g["environment"].as<std::string>();
    if(g["log_level"]) config.global.log_level = g["log_level"].as<std::string>();
    if(g["cpu_affinity"]) config.global.cpu_affinity = g["cpu_affinity"].as<bool>();
    if(g["log_file_path"]) config.global.log_file_path = g["log_file_path"].as<std::string>();
  }

  if(root["threading_matrix"]) {
    auto t = root["threading_matrix"];
    if(t["network_core"]) config.threading_matrix.network_core = t["network_core"].as<uint32_t>();
    if(t["parser_cores"] && t["parser_cores"].IsSequence()) {
      config.threading_matrix.parser_cores.clear();
      for(const auto& c : t["parser_cores"]) config.threading_matrix.parser_cores.push_back(c.as<uint32_t>());
    }
    if(t["storage_core"]) config.threading_matrix.storage_core = t["storage_core"].as<uint32_t>();
    if(t["pub_core"]) config.threading_matrix.pub_core = t["pub_core"].as<uint32_t>();
    if(t["telemetry_core"]) config.threading_matrix.telemetry_core = t["telemetry_core"].as<uint32_t>();
  }

  if(root["telemetry"]) {
    auto t = root["telemetry"];
    if(t["prometheus_enabled"]) config.telemetry.prometheus_enabled = t["prometheus_enabled"].as<bool>();
    if(t["listen_port"]) config.telemetry.listen_port = t["listen_port"].as<uint16_t>();
    if(t["report_interval_ms"]) config.telemetry.report_interval_ms = t["report_interval_ms"].as<uint32_t>();
  }

  if(root["pub"]) {
    auto p = root["pub"];
    if(p["tcp_endpoint"]) config.pub.tcp_endpoint = p["tcp_endpoint"].as<std::string>();
    if(p["ipc_endpoint"]) config.pub.ipc_endpoint = p["ipc_endpoint"].as<std::string>();
  }

  if(root["storage"]) {
    auto s = root["storage"];
    if(s["use_engine"]) config.storage.use_engine = s["use_engine"].as<std::string>();
    if(s["persist_to_disk"]) config.storage.persist_to_disk = s["persist_to_disk"].as<bool>();
    if(s["dolphindb"]) {
      auto d = s["dolphindb"];
      if(d["host"]) config.storage.dolphindb.host = d["host"].as<std::string>();
      if(d["port"]) config.storage.dolphindb.port = d["port"].as<uint16_t>();
      if(d["user"]) config.storage.dolphindb.user = d["user"].as<std::string>();
      if(d["password"]) {
        config.storage.dolphindb.password = SecureString::FromPlain(d["password"].as<std::string>());
        d.remove("password");  // erase plaintext from YAML node cache
      }
      if(d["buffer_size"]) config.storage.dolphindb.buffer_size = d["buffer_size"].as<uint32_t>();
      if(d["flush_interval_ms"]) config.storage.dolphindb.flush_interval_ms = d["flush_interval_ms"].as<uint32_t>();
      // MTW writer parameters
      if(d["mtw_batch_size"]) config.storage.dolphindb.mtw_batch_size = d["mtw_batch_size"].as<int>();
      if(d["mtw_throttle_sec"]) config.storage.dolphindb.mtw_throttle_sec = d["mtw_throttle_sec"].as<float>();
      if(d["mtw_thread_count"]) config.storage.dolphindb.mtw_thread_count = d["mtw_thread_count"].as<int>();
      // Auto-schema and health check
      if(d["auto_init_schema"]) config.storage.dolphindb.auto_init_schema = d["auto_init_schema"].as<bool>();
      if(d["health_check_interval_ms"]) config.storage.dolphindb.health_check_interval_ms = d["health_check_interval_ms"].as<uint32_t>();
      // DFS partition config
      if(d["dfs_db_path"]) config.storage.dolphindb.dfs_db_path = d["dfs_db_path"].as<std::string>();
      if(d["hash_buckets"]) config.storage.dolphindb.hash_buckets = d["hash_buckets"].as<int>();
      if(d["partition_granularity"]) config.storage.dolphindb.partition_granularity = d["partition_granularity"].as<std::string>();
    }
    if(s["mmap"]) {
      auto m = s["mmap"];
      if(m["output_path"]) config.storage.mmap.output_path = m["output_path"].as<std::string>();
      if(m["sync_interval_records"]) config.storage.mmap.sync_interval_records = m["sync_interval_records"].as<uint32_t>();
    }
    if(s["csv"]) {
      auto csv_node = s["csv"];
      if(csv_node["output_path"]) config.storage.csv.output_path = csv_node["output_path"].as<std::string>();
    }
  }

  if(root["rithmic"]) {
    config.rithmic = ParseRithmicConfig(root["rithmic"]);
  }

  if(root["exchanges"] && root["exchanges"].IsSequence())
    for(const auto& ex : root["exchanges"]) config.exchanges.push_back(ParseExchange(ex));

  return config;
}

RootConfig Config::instance_;

void Config::Load(const std::string& path) { instance_ = LoadConfig(path); }
const RootConfig& Config::Instance() { return instance_; }

}  // namespace sqc
