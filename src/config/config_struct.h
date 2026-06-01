#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
  std::string password = "123";
  uint32_t buffer_size = 2000;
  uint32_t flush_interval_ms = 10;
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
  std::vector<ExchangeConfig> exchanges;
};

}  // namespace sqc
