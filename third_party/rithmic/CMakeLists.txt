cmake_minimum_required(VERSION 3.20)
project(rithmic_api LANGUAGES CXX)

# Rithmic SDK provides pre-compiled static libraries for Linux x86_64 only.
# On other architectures, or when the .a files are not present, we provide a
# header-only INTERFACE target so code referencing RApi symbols can still
# compile (linking will fail without the matching static libraries — deploy on
# x86_64 with the full SDK for Rithmic integration).
set(RITHMIC_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
	set(RITHMIC_LIB_ARCH_DIR "${RITHMIC_LIB_DIR}/linux")
	set(RITHMIC_PRIMARY_LIB "${RITHMIC_LIB_ARCH_DIR}/libRApiPlus-optimize.a")

	if(EXISTS "${RITHMIC_PRIMARY_LIB}")
		set(SQC_WITH_RITHMIC ON CACHE INTERNAL "Rithmic static libraries available")
		message(STATUS "Rithmic SDK: Linux x86_64 — static libraries enabled")

		add_library(rithmic_api STATIC IMPORTED GLOBAL)
		set_target_properties(rithmic_api PROPERTIES
			IMPORTED_LOCATION "${RITHMIC_PRIMARY_LIB}"
			INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
		)
		target_link_libraries(rithmic_api INTERFACE
			"${RITHMIC_LIB_ARCH_DIR}/libOmneStreamEngine-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/libOmneChannel-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/libOmneEngine-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/lib_api-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/lib_apipoll-stubs-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/lib_kit-optimize.a"
			# libcrypto.a / libssl.a are NOT listed here —
			# the project already links OpenSSL::Crypto / OpenSSL::SSL from Conan,
			# and linking the Rithmic-bundled copies causes duplicate symbol errors.
			pthread dl rt z
		)
	else()
		set(SQC_WITH_RITHMIC OFF CACHE INTERNAL "Rithmic static libraries available")
		message(STATUS "Rithmic SDK: Linux x86_64 — static libraries NOT found at ${RITHMIC_PRIMARY_LIB} (headers only)")

		add_library(rithmic_api INTERFACE)
		target_include_directories(rithmic_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
	set(RITHMIC_LIB_ARCH_DIR "${RITHMIC_LIB_DIR}/darwin")
	set(RITHMIC_PRIMARY_LIB "${RITHMIC_LIB_ARCH_DIR}/libRApiPlus-optimize.a")

	if(EXISTS "${RITHMIC_PRIMARY_LIB}")
		set(SQC_WITH_RITHMIC ON CACHE INTERNAL "Rithmic static libraries available")
		message(STATUS "Rithmic SDK: macOS x86_64 — static libraries enabled")

		add_library(rithmic_api STATIC IMPORTED GLOBAL)
		set_target_properties(rithmic_api PROPERTIES
			IMPORTED_LOCATION "${RITHMIC_PRIMARY_LIB}"
			INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
		)
		target_link_libraries(rithmic_api INTERFACE
			"${RITHMIC_LIB_ARCH_DIR}/libOmneStreamEngine-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/libOmneChannel-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/libOmneEngine-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/lib_api-optimize.a"
			"${RITHMIC_LIB_ARCH_DIR}/lib_kit-optimize.a"
			pthread dl z
		)
	else()
		set(SQC_WITH_RITHMIC OFF CACHE INTERNAL "Rithmic static libraries available")
		message(STATUS "Rithmic SDK: macOS x86_64 — static libraries NOT found at ${RITHMIC_PRIMARY_LIB} (headers only)")

		add_library(rithmic_api INTERFACE)
		target_include_directories(rithmic_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
	endif()
else()
	set(SQC_WITH_RITHMIC OFF CACHE INTERNAL "Rithmic static libraries available")
	message(STATUS "Rithmic SDK: ${CMAKE_SYSTEM_PROCESSOR} — static libraries NOT available (headers only)")

	# Header-only INTERFACE target — allows compilation but won't link.
	# Deploy on x86_64 with full SDK for Rithmic integration.
	add_library(rithmic_api INTERFACE)
	target_include_directories(rithmic_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()

# Copy Rithmic config/cert files to build directory at build time.
# The RApi engine loads these via MML_SSL_CLNT_AUTH_FILE=etc/... (relative path),
# so they must be available at runtime alongside the binary.
add_custom_target(copy_rithmic_etc ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/etc"
        "${CMAKE_BINARY_DIR}/bin/etc"
    COMMENT "Copying Rithmic etc/ → ${CMAKE_BINARY_DIR}/bin/etc"
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/etc" DESTINATION "${CMAKE_BINARY_DIR}/bin/etc")
