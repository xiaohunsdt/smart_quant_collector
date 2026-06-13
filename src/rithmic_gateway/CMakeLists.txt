cmake_minimum_required(VERSION 3.20)

# ============================================================================
# rithmic_gateway — separate process for Rithmic API connection.
#
# Links ONLY against Rithmic-bundled OpenSSL (via rithmic_api target).
# Must NOT link against Conan OpenSSL (OpenSSL::SSL / OpenSSL::Crypto).
# ============================================================================

find_package(yaml-cpp REQUIRED)
find_package(fmt REQUIRED)
find_package(quill CONFIG REQUIRED)

# rithmic_api is a GLOBAL imported target created by root CMakeLists.txt
# with bundled libssl.a + libcrypto.a. However its INTERFACE link order
# puts crypto before ssl, which breaks static linking (ssl depends on crypto).
# We link the Rithmic SDK directly with correct order.
add_executable(rithmic_gateway
    main.cpp
    rithmic_callbacks.cpp
    rithmic_engine.cpp
    "${CMAKE_SOURCE_DIR}/src/config/config_loader.cpp"
)

target_include_directories(rithmic_gateway PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    "${CMAKE_SOURCE_DIR}/third_party/rithmic/include"
)

set(RITHMIC_LIBS "${CMAKE_SOURCE_DIR}/third_party/rithmic/lib/linux")

# NOTE: libssl.a before libcrypto.a (ssl depends on crypto symbols)
# No Conan OpenSSL in this link line.
target_link_libraries(rithmic_gateway PRIVATE
    "${RITHMIC_LIBS}/libRApiPlus-optimize.a"
    "${RITHMIC_LIBS}/libOmneStreamEngine-optimize.a"
    "${RITHMIC_LIBS}/libOmneChannel-optimize.a"
    "${RITHMIC_LIBS}/libOmneEngine-optimize.a"
    "${RITHMIC_LIBS}/lib_api-optimize.a"
    "${RITHMIC_LIBS}/lib_apipoll-stubs-optimize.a"
    "${RITHMIC_LIBS}/lib_kit-optimize.a"
    "${RITHMIC_LIBS}/libssl.a"
    "${RITHMIC_LIBS}/libcrypto.a"
    yaml-cpp::yaml-cpp
    fmt::fmt
    quill::quill
    pthread
    dl
    rt
    z
)

# Set output next to the main binary
set_target_properties(rithmic_gateway PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_dependencies(rithmic_gateway copy_runtime_config)
