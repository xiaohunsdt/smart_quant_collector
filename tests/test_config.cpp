#include <gtest/gtest.h>

#include "src/config/config_loader.h"

namespace sqc {
namespace {

TEST(ConfigLoaderTest, LoadsValidConfig) {
  auto config = LoadConfig("../config/config.yaml");

  EXPECT_EQ(config.global.environment, "production");
  EXPECT_EQ(config.global.log_level, "info");
  EXPECT_TRUE(config.global.cpu_affinity);

  EXPECT_EQ(config.threading_matrix.network_core, 2);
  EXPECT_EQ(config.threading_matrix.parser_cores.size(), 2);
  EXPECT_EQ(config.threading_matrix.parser_cores[0], 3);
  EXPECT_EQ(config.threading_matrix.parser_cores[1], 4);
  EXPECT_EQ(config.threading_matrix.storage_core, 5);
  EXPECT_EQ(config.threading_matrix.pub_core, 6);
  EXPECT_EQ(config.threading_matrix.telemetry_core, 7);

  EXPECT_EQ(config.telemetry.listen_port, 8080);
  EXPECT_TRUE(config.telemetry.prometheus_enabled);

  EXPECT_EQ(config.gateway.unified_pub_endpoint, "tcp://*:5555");
  EXPECT_EQ(config.gateway.internal_router, "ipc:///tmp/gateway_router.ipc");
  EXPECT_EQ(config.gateway.registered_channels.size(), 3);

  EXPECT_EQ(config.storage.use_engine, "csv");
  EXPECT_EQ(config.storage.dolphindb.host, "127.0.0.1");
  EXPECT_EQ(config.storage.dolphindb.port, 8848);

  EXPECT_GE(config.exchanges.size(), 1);
}

TEST(ConfigLoaderTest, BinanceIsDisabled) {
  auto config = LoadConfig("../config/config.yaml");

  bool found_binance = false;
  for (const auto& ex : config.exchanges) {
    if (ex.name == "binance") {
      found_binance = true;
      EXPECT_FALSE(ex.enabled);
    }
  }
  EXPECT_TRUE(found_binance);
}

TEST(ConfigLoaderTest, GateioPerpetualHasSymbols) {
  auto config = LoadConfig("../config/config.yaml");

  bool found_gateio = false;
  for (const auto& ex : config.exchanges) {
    if (ex.name == "gateio") {
      found_gateio = true;
      EXPECT_TRUE(ex.enabled);
      for (const auto& ch : ex.channels) {
        if (ch.type == "perpetual") {
          EXPECT_GE(ch.symbols.size(), 3);
          EXPECT_EQ(ch.symbols[0].name, "BTC_USDT");
        }
      }
    }
  }
  EXPECT_TRUE(found_gateio);
}

}  // namespace
}  // namespace sqc
