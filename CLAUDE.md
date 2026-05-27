# Production Requirements

This is an **HFT-grade** crypto data collector. Every design decision must satisfy:

- **HFT 级低延迟** — hot path latency measured in nanoseconds, no unbounded queues, no contended locks
- **零运行时堆分配 (Zero Runtime Allocation)** — hot path must not touch the heap; all memory pre-allocated at startup
- **内核硬隔离** — CPU affinity, core pinning, `isolcpus`, no OS noise on data-plane cores
- **全链路故障自愈** — every component detects failure and self-heals without human intervention

## Development Discipline

1. **职责单一，粒度最小化** — each module/function does exactly one thing, precisely, correctly
2. **代码质量** — meets professional software engineering standards (C++ Core Guidelines, modern C++20 idioms, no anti-patterns)
3. **方案先行** — before any implementation: write design → review → revise → repeat until PASS, then and only then start coding
4. **审查组制度** — every change goes through review team scrutiny:
   - Code style
   - Code quality
   - Code performance (latency, allocation, cache behavior)
   - 未来函数 (forward-compatibility / future-proofing)
   - Logic correctness
   - Strict cycle: 审查 → 修复 → 再审查 → 再修复 → ... → PASS
5. **测试门槛** — every developer ships test cases with their code:
   - **Coverage ≥ 95%**
   - **100% test pass rate**
   - Tests written before implementation (TDD)

# Build

Conan install runs automatically on first CMake configure via `dependencies.cmake`.

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Release build:
```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

# Test

```bash
ctest --test-dir build --output-on-failure
```

Run a single test binary:
```bash
cmake --build build --target test_gateio_parser
./build/tests/test_gateio_parser
```

# Coverage (LLVM toolchain)

```bash
cmake -B build-cov -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping -O0"
cmake --build build-cov
# Run tests, then:
llvm-profdata merge -sparse default.profraw -o default.profdata
llvm-cov report build-cov/tests/test_* -instr-profile=default.profdata
```

# Lint

```bash
clang-tidy -p build src/**/*.cpp src/**/*.h
```

# Config

Runtime config lives at `config/config.yaml`. Run the collector:

```bash
./build/src/smart_quant_collector config/config.yaml
```

# Stack

C++20, CMake, Conan, GoogleTest, Boost 1.87, OpenSSL 3.4, simdjson, yaml-cpp, ZeroMQ, fmt, quill (logging), prometheus-cpp.

Conan generators: `CMakeDeps`, `CMakeToolchain` (output to `build/conan/`).
