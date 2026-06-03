#include "pub_worker.h"

#include <chrono>
#include <cstring>
#include <thread>

#include "common/logger_init.h"
#include "message_serializer.h"
#include "quill/LogMacros.h"

namespace sqc {

// ── Construction ──────────────────────────────────────────────────

PubWorker::PubWorker(zmq::context_t& ctx, std::vector<std::string> topic_prefixes, size_t num_shards, std::string tcp_endpoint,
                     std::string ipc_endpoint)
    : pub_socket_(ctx, ZMQ_PUB),
      tcp_endpoint_(std::move(tcp_endpoint)),
      ipc_endpoint_(std::move(ipc_endpoint)),
      topic_prefixes_(std::move(topic_prefixes)),
      num_shards_(num_shards) {
  pub_socket_.set(zmq::sockopt::sndhwm, 10000);

  shard_queues_ = std::make_unique<ShardQueues[]>(num_shards_);

  LOG_INFO(GetLogger(), "PubWorker initialized: {} shards, {} channels", num_shards_, topic_prefixes_.size());
}

// ── Publish API (parser threads) ──────────────────────────────────

// Padding must be zeroed by the caller before calling PublishTick.
// The parser callbacks in main.cpp already handle this so we avoid
// a redundant copy + memset on the hot path.
void PubWorker::PublishTick(const TickData& tick, size_t shard) {
  if(shard >= num_shards_) return;

  if(!shard_queues_[shard].tick.try_push(tick)) {
    dropped_queue_count_.fetch_add(1, std::memory_order_relaxed);
  }
}

void PubWorker::PublishDepth(const DepthUpdateEvent& depth, size_t shard) {
  if(shard >= num_shards_) return;

  if(!shard_queues_[shard].depth.try_push(depth)) {
    dropped_queue_count_.fetch_add(1, std::memory_order_relaxed);
  }
}

void PubWorker::PublishBookTicker(const BookTickerEvent& bt, size_t shard) {
  if(shard >= num_shards_) return;

  if(!shard_queues_[shard].book_ticker.try_push(bt)) {
    dropped_queue_count_.fetch_add(1, std::memory_order_relaxed);
  }
}

// ── SendTyped (hot path — template, defined here) ─────────────────

template <typename Serializer>
bool PubWorker::SendTyped(uint32_t channel_id, std::string_view event_suffix, const typename Serializer::value_type& data) {
  // Resolve topic prefix.
  if(channel_id >= topic_prefixes_.size()) return false;
  const auto& prefix = topic_prefixes_[channel_id];
  if(prefix.empty()) return false;

  // Build topic string: "prefix:event_suffix"
  // prefix = "exchange:type:symbol", suffix = "tick"/"depth"/"book_ticker"
  const int topic_len = std::snprintf(topic_buf_, kMaxTopicLen, "%.*s:%.*s", static_cast<int>(prefix.size()), prefix.data(),
                                      static_cast<int>(event_suffix.size()), event_suffix.data());
  if(topic_len < 0 || static_cast<size_t>(topic_len) >= kMaxTopicLen) {
    return false;  // truncated or encoding error
  }

  // Acquire zero-allocation buffer.
  void* payload = buffer_pool_.Acquire();
  if(!payload) {
    dropped_buffer_count_.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  // Compile-time serialization strategy.
  Serializer::serialize(payload, data);

  // Build multi-part ZMQ message.
  zmq::message_t topic_msg(topic_buf_, topic_len);
  zmq::message_t payload_msg(payload, Serializer::wire_size, &ZmqBufferPool::Release, payload);

  // ZMQ guarantees atomic delivery of multi-part messages:
  // if any frame fails (HWM), the entire message is discarded.
  auto r1 = pub_socket_.send(std::move(topic_msg), zmq::send_flags::dontwait | zmq::send_flags::sndmore);
  if(!r1) {
    dropped_hwm_count_.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  auto r2 = pub_socket_.send(std::move(payload_msg), zmq::send_flags::dontwait);
  if(!r2) {
    dropped_hwm_count_.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  return true;
}

// Explicit template instantiations for the 3 message types.
template bool PubWorker::SendTyped<TickSerializer>(uint32_t, std::string_view, const TickData&);
template bool PubWorker::SendTyped<DepthSerializer>(uint32_t, std::string_view, const DepthUpdateEvent&);
template bool PubWorker::SendTyped<BookTickerSerializer>(uint32_t, std::string_view, const BookTickerEvent&);

// ── TrySendOne ────────────────────────────────────────────────────

bool PubWorker::TrySendOne(size_t shard, size_t queue_type) {
  switch(queue_type) {
    case 0: {
      TickData item;
      if(!shard_queues_[shard].tick.try_pop(item)) return false;
      return SendTyped<TickSerializer>(item.channel_id, "tick", item);
    }
    case 1: {
      DepthUpdateEvent item;
      if(!shard_queues_[shard].depth.try_pop(item)) return false;
      return SendTyped<DepthSerializer>(item.channel_id, "depth", item);
    }
    case 2: {
      BookTickerEvent item;
      if(!shard_queues_[shard].book_ticker.try_pop(item)) return false;
      return SendTyped<BookTickerSerializer>(item.channel_id, "book_ticker", item);
    }
    default:
      return false;
  }
}

// ── DispatchCycle ─────────────────────────────────────────────────

void PubWorker::DispatchCycle(size_t& cursor, bool& any_work) {
  const size_t total_queues = num_shards_ * 3;
  for(size_t i = 0; i < total_queues; ++i) {
    const size_t idx = (cursor + i) % total_queues;
    const size_t shard = idx / 3;
    const size_t qtype = idx % 3;
    if(TrySendOne(shard, qtype)) {
      any_work = true;
    }
  }
  cursor = (cursor + 1) % total_queues;
}

// ── DrainAll ──────────────────────────────────────────────────────

void PubWorker::DrainAll() {
  size_t cursor = 0;
  bool any = true;
  while(any) {
    any = false;
    DispatchCycle(cursor, any);
  }
}

// ── Run ───────────────────────────────────────────────────────────

void PubWorker::Run() {
  pub_socket_.bind(tcp_endpoint_);
  pub_socket_.bind(ipc_endpoint_);

  running_.store(true, std::memory_order_relaxed);
  LOG_INFO(GetLogger(), "PubWorker bound to {} and {}", tcp_endpoint_, ipc_endpoint_);

  size_t cursor = 0;
  size_t idle_cycles = 0;
  while(running_.load(std::memory_order_acquire)) {
    bool any_work = false;
    DispatchCycle(cursor, any_work);
    if(!any_work) {
      ++idle_cycles;
      // Yield for the first few idle cycles (sub-µs wakeup latency);
      // fall back to a short sleep after sustained idleness to avoid
      // burning 100 % CPU on the pub core.
      if(idle_cycles > 10) {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
      } else {
        std::this_thread::yield();
      }
    } else {
      idle_cycles = 0;
    }
  }

  DrainAll();
  LOG_INFO(GetLogger(), "PubWorker run loop exited");
}

// ── Stop ──────────────────────────────────────────────────────────

void PubWorker::Stop() { running_.store(false, std::memory_order_release); }

}  // namespace sqc
