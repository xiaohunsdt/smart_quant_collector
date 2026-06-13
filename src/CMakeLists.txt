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
    exchange/exchange_factory.cpp
    exchange/data_dispatcher.cpp
    exchange/crypto/symbol_channel.cpp
    exchange/crypto/shard_parser_worker.cpp
    exchange/crypto/crypto_factory.cpp
    exchange/crypto/binance/binance_common.cpp
    exchange/crypto/binance/binance_spot.cpp
    exchange/crypto/binance/binance_perpetual.cpp
    exchange/crypto/gateio/gateio_common.cpp
    exchange/crypto/gateio/gateio_spot.cpp
    exchange/crypto/gateio/gateio_perpetual.cpp
    storage/mmap_engine.cpp
    storage/storage_router.cpp
    storage/csv_writer.cpp
    pubsub/pub_message.cpp
    pubsub/zmq_buffer_pool.cpp
    pubsub/pub_worker.cpp
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

target_sources(collector_objects PRIVATE storage/dolphindb_client.cpp)
target_link_libraries(collector_objects PUBLIC DolphinDBAPI)

target_sources(collector_objects PRIVATE
    exchange/rithmic/rithmic_receiver.cpp
    exchange/rithmic/rithmic_process_manager.cpp
)
target_compile_definitions(collector_objects PUBLIC
    RITHMIC_GATEWAY_EXE="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rithmic_gateway"
)

# Main executable
add_executable(smart_quant_collector main.cpp)
target_link_libraries(smart_quant_collector PRIVATE collector_objects)
add_dependencies(smart_quant_collector copy_runtime_config)


add_subdirectory(rithmic_gateway)
