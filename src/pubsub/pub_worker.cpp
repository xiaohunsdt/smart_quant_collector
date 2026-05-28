#include "pub_worker.h"

#include <chrono>
#include <cstring>
#include <thread>

#include "config/config_loader.h"
#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

PubWorker::PubWorker() : ctx_(1), pub_socket_(ctx_, ZMQ_PUB) {
  const auto& ep = Config::Instance().gateway.unified_pub_endpoint;
  pub_socket_.set(zmq::sockopt::sndhwm, 10000);
  pub_socket_.bind(ep);
  LOG_INFO(GetLogger(), "PubWorker bound to {}", ep);
}

void PubWorker::Init(size_t num_queues) {
  num_queues_ = num_queues;
  queues_ = std::make_unique<TickSPSCQueue[]>(num_queues);
}

void PubWorker::PublishTick(const TickData& tick, size_t shard_id) {
  if (shard_id >= num_queues_) return;
  if (!queues_[shard_id].try_push(tick)) {
    dropped_count_.fetch_add(1, std::memory_order_relaxed);
  }
}

void PubWorker::Run() {
  running_ = true;
  LOG_INFO(GetLogger(), "PubWorker run loop started");

  TickData tick;
  while (running_.load(std::memory_order_relaxed)) {
    bool any = false;
    for (size_t i = 0; i < num_queues_; ++i) {
      while (queues_[i].try_pop(tick)) {
        zmq::message_t msg(sizeof(TickData));
        std::memcpy(msg.data(), &tick, sizeof(TickData));
        auto res = pub_socket_.send(std::move(msg), zmq::send_flags::dontwait);
        if (!res && errno == EAGAIN) {
          dropped_count_.fetch_add(1, std::memory_order_relaxed);
        }
        any = true;
      }
    }
    if (!any) {
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  }

  // Drain remaining ticks before exit
  for (size_t i = 0; i < num_queues_; ++i) {
    while (queues_[i].try_pop(tick)) {
      zmq::message_t msg(sizeof(TickData));
      std::memcpy(msg.data(), &tick, sizeof(TickData));
      pub_socket_.send(std::move(msg), zmq::send_flags::dontwait);
    }
  }
}

void PubWorker::Stop() { running_ = false; }

}  // namespace sqc
