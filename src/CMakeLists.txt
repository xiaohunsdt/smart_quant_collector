# Object library: all collector sources compiled once, linked into
# the main executable and each test binary (to meet 95% coverage req).
add_library(collector_objects OBJECT
    common/cpu_affinity.cpp
    common/logger_init.cpp
    common/signal_handler.cpp
    config/config_loader.cpp
    exchange/channel_mapping.cpp
    network/ws_client.cpp
    exchange/shard_queue.cpp
    exchange/symbol_channel.cpp
    exchange/shard_parser_worker.cpp
    exchange/exchange_factory.cpp
    exchange/binance/binance_common.cpp
    exchange/binance/binance_spot.cpp
    exchange/binance/binance_perpetual.cpp
    exchange/gateio/gateio_spot.cpp
    exchange/gateio/gateio_perpetual.cpp
    common/http_utils.cpp
    storage/dolphindb_client.cpp
    storage/mmap_engine.cpp
    storage/storage_router.cpp
    storage/csv_writer.cpp
    pubsub/pub_worker.cpp
    pubsub/gateway_router.cpp
    pubsub/gateway_supervisor.cpp
    telemetry/prometheus_exposer.cpp
    telemetry/telemetry_agent.cpp
)

target_include_directories(collector_objects PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(collector_objects PUBLIC
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    simdjson::simdjson
    yaml-cpp::yaml-cpp
    cppzmq
    fmt::fmt
    quill::quill
    prometheus-cpp::core
    prometheus-cpp::pull
)

# Main executable
add_executable(smart_quant_collector main.cpp)
target_link_libraries(smart_quant_collector PRIVATE collector_objects)
