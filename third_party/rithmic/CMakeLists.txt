cmake_minimum_required(VERSION 3.20)
project(rithmic_api LANGUAGES CXX)

# Rithmic SDK provides pre-compiled static libraries for Linux x86_64 only.
# On other architectures, we provide a header-only INTERFACE target so code
# referencing RApi symbols can still compile (linking will fail without the
# matching static libraries — deploy on x86_64 for full integration).
set(RITHMIC_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    set(RITHMIC_LIB_ARCH_DIR "${RITHMIC_LIB_DIR}/linux")
    set(SQC_WITH_RITHMIC ON CACHE INTERNAL "Rithmic static libraries available")
    message(STATUS "Rithmic SDK: Linux x86_64 — static libraries enabled")

    add_library(rithmic_api STATIC IMPORTED GLOBAL)
    set_target_properties(rithmic_api PROPERTIES
        IMPORTED_LOCATION "${RITHMIC_LIB_ARCH_DIR}/libRApiPlus-optimize.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    target_link_libraries(rithmic_api INTERFACE
        "${RITHMIC_LIB_ARCH_DIR}/libOmneStreamEngine-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libOmneChannel-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libOmneEngine-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/lib_api-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/lib_apipoll-stubs-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/lib_kit-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libcrypto.a"
        "${RITHMIC_LIB_ARCH_DIR}/libssl.a"
        pthread dl rt z
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    set(RITHMIC_LIB_ARCH_DIR "${RITHMIC_LIB_DIR}/darwin")
    set(SQC_WITH_RITHMIC ON CACHE INTERNAL "Rithmic static libraries available")
    message(STATUS "Rithmic SDK: macOS x86_64 — static libraries enabled")

    add_library(rithmic_api STATIC IMPORTED GLOBAL)
    set_target_properties(rithmic_api PROPERTIES
        IMPORTED_LOCATION "${RITHMIC_LIB_ARCH_DIR}/libRApiPlus-optimize.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    target_link_libraries(rithmic_api INTERFACE
        "${RITHMIC_LIB_ARCH_DIR}/libOmneStreamEngine-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libOmneChannel-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libOmneEngine-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/lib_api-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/lib_kit-optimize.a"
        "${RITHMIC_LIB_ARCH_DIR}/libcrypto.a"
        "${RITHMIC_LIB_ARCH_DIR}/libssl.a"
        pthread dl z
    )
else()
    set(SQC_WITH_RITHMIC OFF CACHE INTERNAL "Rithmic static libraries available")
    message(STATUS "Rithmic SDK: ${CMAKE_SYSTEM_PROCESSOR} — static libraries NOT available (headers only)")

    # Header-only INTERFACE target — allows compilation but won't link.
    # Deploy on x86_64 for full Rithmic integration.
    add_library(rithmic_api INTERFACE)
    target_include_directories(rithmic_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/etc"
        DESTINATION "${CMAKE_BINARY_DIR}/etc")
