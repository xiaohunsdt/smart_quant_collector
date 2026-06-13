#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <vector>

#include "common/telemetry_slot.h"
#include "exchange/crypto/shard_parser_worker.h"
#include "exchange/crypto/symbol_channel.h"
#include "exchange/exchange_adapter.h"
#include "exchange/shard_queue.h"

namespace sqc {

namespace net = boost::asio;

/// Returns the statically-allocated adapter for (exchange_name, channel_type).
/// Returns nullptr for unknown exchanges — e.g. "rithmic" uses CreateRithmicManager().
const ExchangeAdapter* GetAdapter(std::string_view exchange_name, ChannelType channel_type);

// ---------------------------------------------------------------------------
// CryptoChannels — owns the WebSocket channels and their backing ShardQueues.
// ---------------------------------------------------------------------------

struct CryptoChannels {
  std::vector<std::shared_ptr<ShardQueue>>    shard_queues;
  std::vector<std::shared_ptr<SymbolChannel>> channels;

  void Start() { for (auto& ch : channels) ch->Start(); }
  void Stop()  { for (auto& ch : channels) ch->Stop();  }

  /// Push a poison pill into every shard queue to signal parser workers to stop.
  void DrainQueues() { for (auto& q : shard_queues) q->PushPoisonPill(); }
};

/// Reads Config::Instance(), registers all enabled crypto symbols into
/// ChannelRegistry and StorageRouter, creates SymbolChannels and ShardQueues.
/// Must be called during single-threaded init before any parser threads start.
CryptoChannels BuildCryptoChannels(size_t num_parsers,
                                   net::io_context& io_ctx,
                                   net::ssl::context& ssl_ctx);

// ---------------------------------------------------------------------------
// CryptoParserPool — owns the parser worker threads and their telemetry slots.
// ---------------------------------------------------------------------------

struct CryptoParserPool {
  // Fixed-size array: TelemetrySlot contains std::atomic members (not movable),
  // so std::vector cannot be used here — use a heap array instead.
  std::unique_ptr<TelemetrySlot[]>                telemetry_slots;
  size_t                                          num_slots = 0;
  std::vector<std::unique_ptr<ShardParserWorker>> workers;
  std::vector<std::thread>                        threads;

  /// Start one OS thread per worker. Must be called after BuildParserPool()
  /// and before any shard queues receive messages.
  void Run();

  /// Block until all parser threads have exited. Call after DrainQueues().
  void Shutdown();
};

/// Construct one ShardParserWorker per shard queue in `crypto`, wire each to
/// a DataDispatcher bound to its own TelemetrySlot, and register all slots
/// with TelemetryAgent::Instance().  Must be called before parser threads start.
CryptoParserPool BuildParserPool(const CryptoChannels& crypto);

// ---------------------------------------------------------------------------
// CryptoSubsystem — unified owner of io_ctx, ssl_ctx, channels, and parser pool.
//
// boost::asio::io_context is non-movable, so it is heap-allocated via unique_ptr.
// All members are move-only; CryptoSubsystem itself is movable but not copyable.
// ---------------------------------------------------------------------------

struct CryptoSubsystem {
  std::unique_ptr<net::io_context>   io_ctx;
  std::unique_ptr<net::ssl::context> ssl_ctx;
  CryptoChannels                     channels;
  CryptoParserPool                   parser_pool;

  // Spawn one parser thread per shard.  Call once during startup.
  void RunParsers();

  // Spawn the dedicated network thread (non-blocking).
  // The thread pins itself to network_core, starts WebSocket channels, then
  // drives io_ctx until StopNetwork() signals it to exit.
  void RunNetwork();

  // Signal the network side to stop (call from the shutdown path on main thread).
  void StopNetwork();

  // Push a poison pill into every shard queue so parser threads drain and exit.
  void DrainQueues();

  // Join the network thread and all parser threads.  Call after DrainQueues().
  void Shutdown();

 private:
  std::thread network_thread_;
};

/// Validates parser_cores (logs CRITICAL + returns nullopt if empty), then
/// constructs io_ctx, ssl_ctx, channels, and parser_pool in a single call.
/// Must be called during single-threaded startup before any other threads start.
std::optional<CryptoSubsystem> BuildCryptoSubsystem();

}  // namespace sqc
