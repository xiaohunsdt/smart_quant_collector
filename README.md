# Smart Quant Collector

HFT-grade multi-market data collector. Ingests real-time streams from **crypto exchanges** (Binance, Gate.io — spot + perpetual via WebSocket) and **traditional futures** (CME, COMEX, etc. via Rithmic R|API+), parses trades / orderbooks / book-tickers, persists to CSV, memory-mapped files, or DolphinDB, and publishes to downstream consumers via ZeroMQ PUB socket.

## Architecture

```
┌──────────────┐    ┌──────────────┐
│  Binance WS  │    │  Gate.io WS  │   crypto (spot / perpetual)
└──────┬───────┘    └──────┬───────┘
       └───────────┬───────┘
                   │  WebSocket feeds
            ┌──────▼──────┐
            │  WS Client  │  Boost.Asio + SSL
            └──────┬──────┘
                   │
       ┌───────────▼──────────┐
       │  ShardParserWorker   │  one per core, simdjson
       └───────────┬──────────┘
                   │
┌──────────────────┼──────────────────────────────────────────┐
│                  │                                          │
│  ┌───────────────▼───────────────┐   ┌──────────────────┐  │
│  │       DataDispatcher          │◄──│ RithmicReceiver  │  │
│  │  storage + pub + telemetry    │   │  (forwarder)     │  │
│  └───────────────┬───────────────┘   └────────▲─────────┘  │
│                  │                            │ SHM MPSC   │
│                  │                   ┌────────┴─────────┐  │
│                  │                   │ rithmic_gateway  │  │
│                  │                   │ (child process)  │  │
│                  │                   │ R|API+ callbacks │  │
│                  │                   └──────────────────┘  │
│                  │                          ▲             │
│                  │                   RithmicProcessManager │
│                  │                   spawn / monitor / heal│
└──────────────────┼──────────────────────────────────────────┘
                   │
       ┌───────────▼──────────┐
       │    StorageRouter     │  routes to active engine
       │  ┌────┬──────┬────┐  │
       │  │CSV │ MMAP │DDB│  │
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
| Network I/O | 2 | Boost.Asio event loop, SSL WebSocket (crypto) |
| Parser × N | 3, 4 | JSON parsing, field extraction, routing |
| Storage | 5 | Background flush / DolphinDB health check |
| Pub | 6 | SPSC → ZMQ dispatch, round-robin |
| Telemetry | 7 | Prometheus metrics exposition |
| Rithmic forwarder | 9 | SHM MPSC consumer → `DataDispatcher` |
| Rithmic engine | 8 | Child `rithmic_gateway` process (R|API+ event loop) |

## Directory Layout

```
src/
├── common/              # SPSC queue, tick data, CPU affinity, telemetry slots
├── config/              # YAML config loader and struct definitions
├── exchange/
│   ├── crypto/          # Binance, Gate.io adapters, parser workers, shard queues
│   └── rithmic/         # SHM layout, MPSC queues, receiver, process manager
├── network/             # WebSocket client (Boost.Asio + SSL)
├── orderbook/           # DepthUpdateEvent, BookTickerEvent
├── pubsub/              # ZeroMQ publishing, buffer pool, serialization
├── rithmic_gateway/     # Separate R|API+ child process (Linux only)
├── storage/             # CSV, mmap, DolphinDB backends, storage router
└── telemetry/           # Prometheus metrics exposition
tests/                   # GoogleTest unit tests (13 test binaries)
benchmarks/              # Microbenchmarks (queue, parser)
config/                  # Runtime YAML configuration
scripts/                 # DolphinDB schema, data import, debug helpers
third_party/
├── dolphindb/           # DolphinDB C++ API
└── rithmic/             # Rithmic R|API+ SDK + SSL cert params
```

## Stack

**C++20** · CMake · Conan · GoogleTest · Boost 1.87 · OpenSSL 3.4 · simdjson · yaml-cpp · ZeroMQ (cppzmq) · fmt · quill · prometheus-cpp · libuuid · Rithmic R|API+ (Linux)

**Storage backends**: CSV (daily-rotating) · mmap (memory-mapped files) · DolphinDB (batch insert via official C++ API, Linux only)

## Quick Start

### Prerequisites

```bash
# Conan package manager
pip install conan

# System dependencies (Ubuntu/Debian)
sudo apt install -y cmake ninja-build libboost-all-dev libssl-dev
```

Conan dependencies install automatically on first CMake configure via `cmake/dependencies.cmake`.

### Build

```bash
# Debug
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release (optimized)
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Build outputs land in `build/bin/`:

| Binary | Description |
|--------|-------------|
| `smart_quant_collector` | Main collector process |
| `rithmic_gateway` | Rithmic R|API+ child (auto-spawned when futures exchanges are enabled) |

`config/` and `etc/` (Rithmic SSL cert params) are copied to `build/bin/` at build time.

### Run

```bash
# Edit config/config.yaml first, then run from build/bin/:
cd build/bin
./smart_quant_collector config/config.yaml

# Or from repo root:
./build/bin/smart_quant_collector config/config.yaml
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

### Lint

```bash
clang-tidy -p build src/**/*.cpp src/**/*.h
```

## Configuration

See `config/config.yaml`. Key settings:

```yaml
global:
  cpu_affinity: true
  log_file_path: "./log/collector.log"

threading_matrix:
  network_core: 2
  parser_cores: [3, 4]
  storage_core: 5
  pub_core: 6
  telemetry_core: 7

telemetry:
  prometheus_enabled: true
  listen_port: 8080

pub:
  tcp_endpoint: "tcp://*:5555"
  ipc_endpoint: "ipc:///tmp/collector_pub.ipc"

storage:
  persist_to_disk: true
  use_engine: "csv"       # "csv" | "mmap" | "dolphindb"
  csv:
    output_path: "./trade_data/"
  mmap:
    output_path: "./mmap_cache/"
    sync_interval_records: 10000
  dolphindb:              # Linux only
    host: "127.0.0.1"
    port: 8848
    buffer_size: 2000
    flush_interval_ms: 10
    mtw_batch_size: 20000
    mtw_throttle_sec: 1.0
    mtw_thread_count: 4
    auto_init_schema: true
    health_check_interval_ms: 5000
    dfs_db_path: "dfs://trade_db"
    hash_buckets: 20
    partition_granularity: "day"  # "day" | "month" | "year"

rithmic:                  # Required when any futures exchange is enabled
  user: "your@broker.com"
  password: "..."
  shm_name: "/sqc_rithmic"
  engine_core: 8
  forwarder_core: 9
  ssl_cert_file: "etc/rithmic_ssl_cert_auth_params"
  domain_servers: "..."
  domain_name: "..."
  # See config/config.yaml for full R|API+ connect-point fields

exchanges:
  # Crypto — channel types: spot | perpetual
  - name: "binance"
    enabled: true
    channels:
      - type: spot
        symbols:
          - name: "BTCUSDT"
            depth_level: 10
            record_tick: true

  # Traditional futures — channel type: futures (Rithmic)
  - name: "COMEX"
    enabled: true
    channels:
      - type: futures
        symbols:
          - name: "GC"
            depth_level: 10
            record_tick: true
```

**Channel types**:

| Type | Exchanges | Transport |
|------|-----------|-----------|
| `spot` | Binance, Gate.io | WebSocket |
| `perpetual` | Binance, Gate.io | WebSocket |
| `futures` | CME, COMEX, … | Rithmic R|API+ (child process + SHM) |

Per-symbol flags: `record_tick` toggles trade ingestion; `persist_to_disk` overrides the global `storage.persist_to_disk` default.

### Rithmic Setup

When any enabled exchange uses `type: futures`, the collector:

1. Registers channels and creates POSIX shared-memory MPSC queues (`/sqc_rithmic` by default).
2. Spawns `rithmic_gateway` as a child process (uses Rithmic-bundled OpenSSL, separate from Conan OpenSSL).
3. Runs `RithmicReceiver` on `forwarder_core` to drain tick / depth / book-ticker queues into the same `DataDispatcher` → storage + ZMQ path as crypto.
4. Monitors child heartbeat and auto-restarts on crash or timeout (rate-limited).

The child reads R|API+ credentials and connect points from the `rithmic:` block in `config.yaml`. SSL cert params must be present at `etc/rithmic_ssl_cert_auth_params` (copied to `build/bin/etc/` during build).

### DolphinDB Setup

The collector auto-initializes the DolphinDB schema on first connect (`auto_init_schema: true`):

1. **Database**: `dfs://trade_db` — TSDB engine, composite partitioned by date (`partition_granularity`: day / month / year) + `symbol` (HASH buckets), `sortColumns=[symbol, exchange_timestamp]`
2. **DFS Tables**: `trades`, `orderbook` (Array Vector `DOUBLE[]` for bid/ask prices & sizes), `bookticker`
3. **Stream Tables**: `trades_stream`, `orderbook_stream`, `bookticker_stream` — in-memory write buffers
4. **Subscriptions**: Each stream subscribes to its DFS table via `subscribeTable` (async batch persist)

No manual `.dos` script execution needed. See `scripts/dolphindb_schema.dos` for the reference DDL.

**Migration from old schema** (VALUE exchange + RANGE day, exploded orderbook columns):

- Stop collector → export data from DolphinDB → `dropDatabase("dfs://trade_db")` → restart collector with `auto_init_schema: true` → re-import historical data via `scripts/import_trade_data.py`

## Design Principles

- **HFT low latency** — nanosecond-scale hot path, no unbounded queues
- **Zero heap allocation** — all memory pre-allocated; hot path never touches the heap
- **Kernel isolation** — CPU affinity, core pinning via `isolcpus`
- **Self-healing** — every component detects failure and recovers without human intervention (including Rithmic child restart)
- **TDD** — tests written first, ≥95% coverage, 100% pass rate

## ZMQ Wire Format

Subscribers receive multi-part ZMQ messages:

```
[Frame 1] topic:  "binance:spot:BTCUSDT:tick"
[Frame 2] payload: [1-byte type tag] [raw struct bytes]
```

Type tags: `0x01` = TickData, `0x02` = DepthUpdateEvent, `0x03` = BookTickerEvent

Topics follow `{exchange}:{channel_type}:{symbol}:{event}` — e.g. `COMEX:futures:GC:tick`.
