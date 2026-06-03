# Smart Quant Collector

HFT-grade crypto market data collector. Ingests real-time WebSocket streams from Binance and Gate.io (spot + perpetual), parses trades/orderbooks/book-tickers, persists to CSV or memory-mapped files or DolphinDB, and publishes to downstream consumers via ZeroMQ PUB socket.

## Architecture

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  Binance WS  │    │  Gate.io WS  │    │  (more TBD)  │
└──────┬───────┘    └──────┬───────┘    └──────┬───────┘
       │                   │                   │
       └───────────┬───────┴───────────────────┘
                   │  WebSocket feeds
            ┌──────▼──────┐
            │  WS Client  │  Boost.Asio + SSL
            └──────┬──────┘
                   │
       ┌───────────▼──────────┐
       │  ShardParserWorker   │  one per core
       │  ┌────────────────┐  │  simdjson, lock-free
       │  │ per-exchange   │  │
       │  │ adapter        │  │
       │  └───────┬────────┘  │
       └──────────┼───────────┘
                  │
       ┌──────────▼───────────┐
       │    StorageRouter     │  routes to active engine
       │  ┌────┬──────┬────┐  │
       │  │CSV │ MMAP │DDB│  │  (mutex-guarded)
       │  └────┴──────┴────┘  │
       └──────────┬───────────┘
                  │
       ┌──────────▼───────────┐
       │     PubWorker        │  lock-free SPSC → ZMQ PUB
       │  SPSC per shard ×3   │  zero-copy ZMQ buffer pool
       └──────────┬───────────┘
                  │  tcp://*:5555 / ipc:///tmp/collector_pub.ipc
       ┌──────────▼───────────┐
       │  Downstream consumers │
       └──────────────────────┘
```

**Thread model** (CPU-pinned via `isolcpus`):

| Thread | Core (default) | Role |
|--------|---------------|------|
| Network I/O | 2 | Boost.Asio event loop, SSL WebSocket |
| Parser × N | 3, 4 | JSON parsing, field extraction, routing |
| Storage | 5 | Background flush / DolphinDB health check |
| Pub | 6 | SPSC → ZMQ dispatch, round-robin |
| Telemetry | 7 | Prometheus metrics exposition |

## Directory Layout

```
src/
├── common/          # Shared utilities (SPSC queue, tick data, CPU affinity, telemetry)
├── config/          # YAML config loader and struct definitions
├── exchange/        # Exchange adapters (Binance, Gate.io), parser workers, shard queues
├── network/         # WebSocket client (Boost.Asio + SSL)
├── orderbook/       # Order book event types (DepthUpdateEvent, BookTickerEvent)
├── pubsub/          # ZeroMQ publishing, buffer pool, message serialization
├── storage/         # Storage backends (CSV, mmap, DolphinDB), storage router
└── telemetry/       # Prometheus metrics exposition
tests/               # GoogleTest unit tests (11 test binaries)
benchmarks/          # Microbenchmarks (queue, parser)
config/              # Runtime YAML configuration
third_party/         # Vendored SDKs (DolphinDB C++ API)
```

## Stack

**C++20** · CMake · Conan · GoogleTest · Boost 1.87 · OpenSSL 3.4 · simdjson · yaml-cpp · ZeroMQ (cppzmq) · fmt · quill · prometheus-cpp

**Storage backends**: CSV (daily-rotating) · mmap (memory-mapped files) · DolphinDB (batch insert via official C++ API, Linux only)

## Quick Start

### Prerequisites

```bash
# Conan package manager
pip install conan

# System dependencies (Ubuntu/Debian)
sudo apt install -y cmake ninja-build libboost-all-dev libssl-dev
```

### Build

```bash
# Debug
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release (optimized)
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
# Edit config/config.yaml first, then:
./build/src/smart_quant_collector config/config.yaml
```

### Test

```bash
# All tests
ctest --test-dir build --output-on-failure

# Single test
cmake --build build --target test_gateio_parser
./build/tests/test_gateio_parser
```

### Coverage (LLVM)

```bash
cmake -B build-cov -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping -O0"
cmake --build build-cov
# Run tests, then:
llvm-profdata merge -sparse default.profraw -o default.profdata
llvm-cov report build-cov/tests/test_* -instr-profile=default.profdata
```

## Configuration

See `config/config.yaml`. Key settings:

```yaml
storage:
  use_engine: "csv"       # "csv" | "mmap" | "dolphindb"
  csv:
    output_path: "./trade_data/"
  dolphindb:              # Linux only
    host: "127.0.0.1"
    port: 8848
    buffer_size: 2000         # TickData double-buffer (rows)
    flush_interval_ms: 10     # Storage thread sleep interval
    mtw_batch_size: 20000     # MTW internal batch size
    mtw_throttle_sec: 1.0     # MTW flush throttle (seconds)
    mtw_thread_count: 4       # MTW worker threads per writer
    auto_init_schema: true    # Auto-create DB/tables/streams on connect
    health_check_interval_ms: 5000  # Ping interval (0=disable)
    dfs_db_path: "dfs://trade_db"
    hash_buckets: 20          # HASH partition buckets for symbol

exchanges:
  - name: "binance"
    enabled: true
    channels:
      - type: spot
        symbols:
          - name: "BTCUSDT"
            depth_level: 10    # Per-symbol orderbook depth from config
```

### DolphinDB Setup

The collector auto-initializes the DolphinDB schema on first connect (`auto_init_schema: true`):

1. **Database**: `dfs://trade_db` — TSDB engine, composite partitioned by `trade_date` (RANGE daily) + `symbol` (HASH 20 buckets), `sortColumns=[symbol, exchange_timestamp]`
2. **DFS Tables**: `trades`, `orderbook` (Array Vector `DOUBLE[]` for bid/ask prices & sizes), `bookticker`
3. **Stream Tables**: `trades_stream`, `orderbook_stream`, `bookticker_stream` — in-memory write buffers
4. **Subscriptions**: Each stream subscribes to its DFS table via `subscribeTable` (async batch persist)

No manual `.dos` script execution needed. See `dolphindb_schema.dos` for the reference DDL.

**Migration from old schema** (VALUE exchange + RANGE day, exploded orderbook columns):
- Stop collector → export data from DolphinDB → `dropDatabase("dfs://trade_db")` → restart collector with `auto_init_schema: true` → re-import historical data via `import_trade_data.py`

## Design Principles

- **HFT low latency** — nanosecond-scale hot path, no unbounded queues
- **Zero heap allocation** — all memory pre-allocated; hot path never touches the heap
- **Kernel isolation** — CPU affinity, core pinning via `isolcpus`
- **Self-healing** — every component detects failure and recovers without human intervention
- **TDD** — tests written first, ≥95% coverage, 100% pass rate

## ZMQ Wire Format

Subscribers receive multi-part ZMQ messages:

```
[Frame 1] topic:  "binance:spot:BTCUSDT:tick"
[Frame 2] payload: [1-byte type tag] [raw struct bytes]
```

Type tags: `0x01` = TickData, `0x02` = DepthUpdateEvent, `0x03` = BookTickerEvent
