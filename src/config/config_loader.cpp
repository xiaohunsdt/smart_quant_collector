#include "config_loader.h"

#include <stdexcept>

#include "yaml-cpp/yaml.h"

namespace sqc {

namespace {

SymbolConfig ParseSymbol(const YAML::Node& node) {
  SymbolConfig s;
  s.name = node["name"].as<std::string>();
  if (node["enabled"]) s.enabled = node["enabled"].as<bool>();
  if (node["depth_level"]) s.depth_level = node["depth_level"].as<uint32_t>();
  if (node["record_tick"]) s.record_tick = node["record_tick"].as<bool>();
  if (node["persist_to_disk"]) s.persist_to_disk = node["persist_to_disk"].as<bool>();
  return s;
}

ChannelConfig ParseChannel(const YAML::Node& node) {
  ChannelConfig c;
  c.type = node["type"].as<std::string>();
  if (node["symbols"] && node["symbols"].IsSequence()) {
    for (const auto& sym : node["symbols"]) {
      c.symbols.push_back(ParseSymbol(sym));
    }
  }
  return c;
}

ExchangeConfig ParseExchange(const YAML::Node& node) {
  ExchangeConfig e;
  e.name = node["name"].as<std::string>();
  if (node["enabled"]) e.enabled = node["enabled"].as<bool>();
  if (node["channels"] && node["channels"].IsSequence()) {
    for (const auto& ch : node["channels"]) {
      e.channels.push_back(ParseChannel(ch));
    }
  }
  return e;
}

}  // namespace

RootConfig LoadConfig(const std::string& path) {
  YAML::Node root = YAML::LoadFile(path);
  RootConfig config;

  if (root["global"]) {
    auto g = root["global"];
    if (g["environment"]) config.global.environment = g["environment"].as<std::string>();
    if (g["log_level"]) config.global.log_level = g["log_level"].as<std::string>();
    if (g["cpu_affinity"]) config.global.cpu_affinity = g["cpu_affinity"].as<bool>();
    if (g["log_file_path"]) config.global.log_file_path = g["log_file_path"].as<std::string>();
  }

  if (root["threading_matrix"]) {
    auto t = root["threading_matrix"];
    if (t["network_core"]) config.threading_matrix.network_core = t["network_core"].as<uint32_t>();
    if (t["parser_cores"] && t["parser_cores"].IsSequence()) {
      config.threading_matrix.parser_cores.clear();
      for (const auto& c : t["parser_cores"])
        config.threading_matrix.parser_cores.push_back(c.as<uint32_t>());
    }
    if (t["storage_core"]) config.threading_matrix.storage_core = t["storage_core"].as<uint32_t>();
    if (t["pub_core"]) config.threading_matrix.pub_core = t["pub_core"].as<uint32_t>();
    if (t["telemetry_core"]) config.threading_matrix.telemetry_core = t["telemetry_core"].as<uint32_t>();
  }

  if (root["telemetry"]) {
    auto t = root["telemetry"];
    if (t["prometheus_enabled"]) config.telemetry.prometheus_enabled = t["prometheus_enabled"].as<bool>();
    if (t["listen_port"]) config.telemetry.listen_port = t["listen_port"].as<uint16_t>();
    if (t["report_interval_ms"]) config.telemetry.report_interval_ms = t["report_interval_ms"].as<uint32_t>();
  }

  if (root["pub"]) {
    auto p = root["pub"];
    if (p["tcp_endpoint"]) config.pub.tcp_endpoint = p["tcp_endpoint"].as<std::string>();
    if (p["ipc_endpoint"]) config.pub.ipc_endpoint = p["ipc_endpoint"].as<std::string>();
  }

  if (root["storage"]) {
    auto s = root["storage"];
    if (s["use_engine"]) config.storage.use_engine = s["use_engine"].as<std::string>();
    if (s["persist_to_disk"]) config.storage.persist_to_disk = s["persist_to_disk"].as<bool>();
    if (s["dolphindb"]) {
      auto d = s["dolphindb"];
      if (d["host"]) config.storage.dolphindb.host = d["host"].as<std::string>();
      if (d["port"]) config.storage.dolphindb.port = d["port"].as<uint16_t>();
      if (d["user"]) config.storage.dolphindb.user = d["user"].as<std::string>();
      if (d["password"])
        config.storage.dolphindb.password = SecureString::FromPlain(d["password"].as<std::string>());
      if (d["buffer_size"]) config.storage.dolphindb.buffer_size = d["buffer_size"].as<uint32_t>();
      if (d["flush_interval_ms"]) config.storage.dolphindb.flush_interval_ms = d["flush_interval_ms"].as<uint32_t>();
    }
    if (s["mmap"]) {
      auto m = s["mmap"];
      if (m["output_path"]) config.storage.mmap.output_path = m["output_path"].as<std::string>();
      if (m["sync_interval_records"]) config.storage.mmap.sync_interval_records = m["sync_interval_records"].as<uint32_t>();
    }
    if (s["csv"]) {
      auto csv_node = s["csv"];
      if (csv_node["output_path"]) config.storage.csv.output_path = csv_node["output_path"].as<std::string>();
    }
  }

  if (root["exchanges"] && root["exchanges"].IsSequence())
    for (const auto& ex : root["exchanges"])
      config.exchanges.push_back(ParseExchange(ex));

  return config;
}

RootConfig Config::instance_;

void Config::Load(const std::string& path) { instance_ = LoadConfig(path); }
const RootConfig& Config::Instance() { return instance_; }

}  // namespace sqc
