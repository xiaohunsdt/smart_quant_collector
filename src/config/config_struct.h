#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "secure_string.h"

namespace sqc {

struct GlobalConfig {
  std::string environment = "production";
  std::string log_level = "info";
  bool cpu_affinity = true;
  std::string log_file_path = "./log/collector.log";
};

struct ThreadingConfig {
  uint32_t network_core = 2;
  std::vector<uint32_t> parser_cores = {3, 4};
  uint32_t storage_core = 5;
  uint32_t pub_core = 6;
  uint32_t telemetry_core = 7;
};

struct TelemetryConfig {
  bool prometheus_enabled = true;
  uint16_t listen_port = 8080;
  uint32_t report_interval_ms = 1000;
};

struct PubConfig {
  std::string tcp_endpoint = "tcp://*:5555";
  std::string ipc_endpoint = "ipc:///tmp/collector_pub.ipc";
};

struct DolphinDBConfig {
  std::string host = "127.0.0.1";
  uint16_t port = 8848;
  std::string user = "admin";
  SecureString password;
  uint32_t buffer_size = 2000;
  uint32_t flush_interval_ms = 10;

  // MTW writer parameters
  int mtw_batch_size = 20000;
  float mtw_throttle_sec = 1.0f;
  int mtw_thread_count = 4;

  // Auto-schema initialization
  bool auto_init_schema = true;
  uint32_t health_check_interval_ms = 5000;  // 0 = disable

  // DFS partition config
  std::string dfs_db_path = "dfs://trade_db";
  int hash_buckets = 20;
  std::string partition_granularity = "day";  // "day" | "month" | "year"
};

struct MmapConfig {
  std::string output_path = "./mmap_cache/";
  uint32_t sync_interval_records = 10000;
};

struct CsvConfig {
  std::string output_path = "./trade_data/";
};

struct StorageConfig {
  std::string use_engine = "csv";
  bool persist_to_disk = true;
  DolphinDBConfig dolphindb;
  MmapConfig mmap;
  CsvConfig csv;
};

struct SymbolConfig {
  std::string name;
  bool enabled = true;
  uint32_t depth_level = 5;
  bool record_tick = true;
  bool persist_to_disk = true;
};

struct ChannelConfig {
  std::string type;  // "spot" or "perpetual"
  std::vector<SymbolConfig> symbols;
};

// Sync with ~/Documents/rithmic_md_saver/config.json
struct RithmicConfig {
  std::string user;
  SecureString password;
  std::string shm_name = "/sqc_rithmic";
  uint32_t engine_core = 8;
  uint32_t forwarder_core = 9;

  // Environment injection (MML_* vars)
  std::string ssl_cert_file = "etc/rithmic_ssl_cert_auth_params";
  std::string domain_servers;
  std::string domain_name;
  std::string license_servers;
  std::string local_broker;
  std::string logger_servers;

  // REngineParams
  std::string app_name = "jewa:SmartQuant";
  std::string app_version = "1.0.0.0";

  // Login connect points
  std::string repository_connect_pt = "login_agent_repositoryc";
  std::string md_connect_pt = "login_agent_tpc";
  std::string md_connect_pt_agg;
  bool use_aggregated_md = false;
  std::string ih_connect_pt;
  std::string ts_connect_pt = "login_agent_opc";
  std::string pnl_connect_pt;
};

struct ExchangeConfig {
  std::string name;
  bool enabled = false;
  std::vector<ChannelConfig> channels;
};

struct RootConfig {
  GlobalConfig global;
  ThreadingConfig threading_matrix;
  TelemetryConfig telemetry;
  PubConfig pub;
  StorageConfig storage;
  RithmicConfig rithmic;
  std::vector<ExchangeConfig> exchanges;
};

}  // namespace sqc
