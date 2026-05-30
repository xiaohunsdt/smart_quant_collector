#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# smart_quant_collector build script
# =============================================================================

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
RELEASE_DIR="${PROJECT_ROOT}/build-release"
COV_DIR="${PROJECT_ROOT}/build-cov"
BIN_DIR="${PROJECT_ROOT}/bin"
LIB_DIR="${PROJECT_ROOT}/lib"
CMAKE_GENERATOR=""

# ---------------------------------------------------------------------------
# Color helpers (auto-disabled when not a terminal)
# ---------------------------------------------------------------------------
if [[ -t 1 ]]; then
    C_RESET='\033[0m'
    C_RED='\033[0;31m'
    C_GREEN='\033[0;32m'
    C_YELLOW='\033[0;33m'
    C_CYAN='\033[0;36m'
else
    C_RESET='' C_RED='' C_GREEN='' C_YELLOW='' C_CYAN=''
fi

info()    { printf "${C_CYAN}[INFO]${C_RESET} %s\n" "$*"; }
success() { printf "${C_GREEN}[OK]${C_RESET}   %s\n" "$*"; }
warn()    { printf "${C_YELLOW}[WARN]${C_RESET} %s\n" "$*" >&2; }
error()   { printf "${C_RED}[ERROR]${C_RESET} %s\n" "$*" >&2; }

# ---------------------------------------------------------------------------
# detect_generator — prefer Ninja, fall back to Unix Makefiles
# ---------------------------------------------------------------------------
detect_generator() {
    if command -v ninja &>/dev/null; then
        CMAKE_GENERATOR="Ninja"
    else
        CMAKE_GENERATOR="Unix Makefiles"
        warn "Ninja not found — falling back to ${CMAKE_GENERATOR}"
    fi
}

# ---------------------------------------------------------------------------
# detect_parallel — number of logical CPUs
# ---------------------------------------------------------------------------
detect_parallel() {
    if command -v nproc &>/dev/null; then
        echo "$(nproc)"
    else
        sysctl -n hw.logicalcpu 2>/dev/null || echo 4
    fi
}

# ---------------------------------------------------------------------------
# find_coverage_tool — resolve llvm-profdata / llvm-cov
# ---------------------------------------------------------------------------
find_coverage_tool() {
    local name="$1"
    if command -v "${name}" &>/dev/null; then
        echo "${name}"
    elif [[ "$(uname -s)" == "Darwin" ]] && command -v xcrun &>/dev/null; then
        echo "xcrun ${name}"
    else
        error "Coverage tool '${name}' not found on PATH (install LLVM / Xcode CLT)"
        return 1
    fi
}

# ---------------------------------------------------------------------------
# cmake_configure — shared configure logic
# ---------------------------------------------------------------------------
cmake_configure() {
    local dir="$1" build_type="$2"; shift 2
    detect_generator
    info "Configuring (${build_type}) in ${dir} ..."
    cmake -B "${dir}" -S "${PROJECT_ROOT}" \
        -G "${CMAKE_GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${build_type}" \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${BIN_DIR}" \
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${LIB_DIR}" \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${LIB_DIR}" \
        "$@"
    success "Configuration complete (${build_type})"
}

# ---------------------------------------------------------------------------
# cmake_build — shared build logic
# ---------------------------------------------------------------------------
cmake_build() {
    local dir="$1" label="$2"; shift 2
    if [[ ! -d "${dir}" ]]; then
        error "Build directory '${dir}' does not exist."
        error "Run './build.sh configure' or './build.sh all' first."
        return 1
    fi
    local parallel; parallel="$(detect_parallel)"
    info "Building (${label}) in ${dir} [${parallel} jobs] ..."
    cmake --build "${dir}" -j "${parallel}" "$@"
    success "Build complete (${label})"
}

# =============================================================================
# Command functions
# =============================================================================

cmd_configure() {
    cmake_configure "${BUILD_DIR}" "Debug" "$@"
}

cmd_build() {
    if [[ $# -gt 0 ]]; then
        cmake_build "${BUILD_DIR}" "Debug" --target "$1"
    else
        cmake_build "${BUILD_DIR}" "Debug"
    fi
}

cmd_test() {
    if [[ ! -d "${BUILD_DIR}" ]]; then
        error "Build directory '${BUILD_DIR}' does not exist."
        error "Run './build.sh configure' or './build.sh all' first."
        return 1
    fi
    local parallel; parallel="$(detect_parallel)"
    info "Building tests in ${BUILD_DIR} ..."
    cmake --build "${BUILD_DIR}" -j "${parallel}"
    info "Running tests ..."
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
    success "All tests passed"
}

cmd_clean() {
    for d in "${BUILD_DIR}" "${RELEASE_DIR}" "${COV_DIR}" "${BIN_DIR}" "${LIB_DIR}"; do
        if [[ -d "$d" ]]; then
            info "Removing ${d} ..."
            rm -rf "$d"
        fi
    done
    rm -f "${PROJECT_ROOT}"/default.profraw "${PROJECT_ROOT}"/*.profdata
    success "Clean complete"
}

cmd_all() {
    cmake_configure "${BUILD_DIR}" "Debug" "$@"
    cmake_build "${BUILD_DIR}" "Debug"
}

cmd_release() {
    cmake_configure "${RELEASE_DIR}" "Release"
    cmake_build "${RELEASE_DIR}" "Release"
    info "Release binary: ${BIN_DIR}/smart_quant_collector"
}

cmd_coverage() {
    # 1. Configure with coverage flags
    cmake_configure "${COV_DIR}" "Debug" \
        -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping -O0 -g"

    # 2. Build
    cmake_build "${COV_DIR}" "Coverage"

    # 3. Run tests, collect profraw into build-cov/
    info "Running tests for coverage data ..."
    LLVM_PROFILE_FILE="${COV_DIR}/coverage-%p.profraw" \
        ctest --test-dir "${COV_DIR}" --output-on-failure

    # 4. Resolve tools
    local profdata_cmd; profdata_cmd="$(find_coverage_tool llvm-profdata)"
    local cov_cmd;      cov_cmd="$(find_coverage_tool llvm-cov)"

    # 5. Merge profile data
    info "Merging profile data ..."
    ${profdata_cmd} merge -sparse "${COV_DIR}"/coverage-*.profraw \
        -o "${COV_DIR}/coverage.profdata"

    # 6. Discover test binaries
    local test_bins=()
    while IFS= read -r f; do
        test_bins+=("$f")
    done < <(find "${COV_DIR}/tests" -type f -executable -name "test_*" 2>/dev/null | sort)

    if [[ ${#test_bins[@]} -eq 0 ]]; then
        # Fallback: tests may be in bin/ if RUNTIME_OUTPUT_DIRECTORY redirected them
        while IFS= read -r f; do
            test_bins+=("$f")
        done < <(find "${BIN_DIR}" -type f -executable -name "test_*" 2>/dev/null | sort)
    fi

    # 7. Report
    info "Coverage report:"
    ${cov_cmd} report "${test_bins[@]}" \
        -instr-profile="${COV_DIR}/coverage.profdata" \
        --ignore-filename-regex="(build|_deps|\.h)" \
        --show-region-summary=false

    success "Coverage report complete"
}

cmd_help() {
    cat <<EOF
Usage: ./build.sh <command> [args...]

Commands:
  configure [cmake-args]   CMake configure (Debug, Ninja)
  build [target]           Build Debug (optional target, e.g. test_gateio_parser)
  test                     Build all + run CTest
  clean                    Remove build/, build-release/, build-cov/, bin/, lib/
  all [cmake-args]         configure + build (Debug)
  coverage                 Coverage build → test → report (LLVM)
  release                  Configure + build Release
  help                     Show this help

Examples:
  ./build.sh all                        # Full debug build
  ./build.sh release                    # Release build
  ./build.sh build test_gateio_parser   # Build single test target
  ./build.sh test                       # Run all tests
  ./build.sh coverage                   # Generate coverage report
  ./build.sh clean                      # Clean everything

Output directories:
  bin/       Executables (smart_quant_collector, test_*)
  lib/       Static libraries
EOF
}

# =============================================================================
# Dispatch
# =============================================================================
main() {
    if [[ $# -eq 0 ]]; then
        cmd_help
        exit 0
    fi
    local cmd="$1"; shift
    case "${cmd}" in
        configure)  cmd_configure "$@" ;;
        build)      cmd_build "$@" ;;
        test)       cmd_test ;;
        clean)      cmd_clean ;;
        all)        cmd_all "$@" ;;
        coverage)   cmd_coverage ;;
        release)    cmd_release ;;
        help|--help|-h) cmd_help ;;
        *)
            error "Unknown command: ${cmd}"
            echo
            cmd_help
            exit 1
            ;;
    esac
}
main "$@"
