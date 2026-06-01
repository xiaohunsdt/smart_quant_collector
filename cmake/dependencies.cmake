# Auto-run conan install on first CMake configure.
# Skip if CONAN_AUTO_INSTALL is explicitly set to OFF, or if the
# conan toolchain already exists (idempotent).

if(NOT DEFINED CONAN_AUTO_INSTALL OR CONAN_AUTO_INSTALL)
    set(_CONAN_TOOLCHAIN "${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake")
    if(NOT EXISTS "${_CONAN_TOOLCHAIN}")
        find_program(CONAN_CMD conan REQUIRED)
        message(STATUS "Running conan install (one-time setup)... ${CMAKE_BUILD_TYPE}")
        execute_process(
            COMMAND ${CONAN_CMD} install ${CMAKE_SOURCE_DIR}
                -of ${CMAKE_BINARY_DIR}
                --build=missing
                -s build_type=${CMAKE_BUILD_TYPE}
            RESULT_VARIABLE CONAN_RESULT
        )
        if(NOT CONAN_RESULT EQUAL 0)
            message(FATAL_ERROR "conan install failed (exit code ${CONAN_RESULT})")
        endif()
        message(STATUS "Conan install complete.")
    endif()
    include("${_CONAN_TOOLCHAIN}")

    # Conan may set CMAKE_OSX_SYSROOT to a bare SDK name (e.g. "macosx")
    # which doesn't work with Unix Makefiles — resolve it to a full path.
    if(APPLE AND NOT CMAKE_GENERATOR MATCHES "Xcode")
        execute_process(
            COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path
            OUTPUT_VARIABLE _SDK_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_SDK_PATH)
            set(CMAKE_OSX_SYSROOT "${_SDK_PATH}" CACHE STRING "" FORCE)
            message(STATUS "Conan toolchain: resolved SDK path to ${_SDK_PATH}")
        endif()
    endif()
endif()
