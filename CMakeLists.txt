cmake_minimum_required(VERSION 3.22)
project(smart_quant_collector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories: executables → bin/, libraries → lib/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

include(cmake/dependencies.cmake)

find_package(Boost CONFIG REQUIRED COMPONENTS thread)
find_package(OpenSSL REQUIRED)
find_package(simdjson REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(fmt REQUIRED)
find_package(quill CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(libuuid REQUIRED CONFIG)

add_subdirectory(third_party/rithmic)
add_subdirectory(third_party/dolphindb)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)

# Copy config/ next to executables so default paths (config/config.yaml) work when cwd is bin/.
add_custom_target(copy_runtime_config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "${CMAKE_BINARY_DIR}/bin/config"
    COMMENT "Copying config/ → ${CMAKE_BINARY_DIR}/bin/config"
)