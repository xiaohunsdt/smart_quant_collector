#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <zmq.hpp>

#include "src/common/tick_data.h"

namespace sqc {

// ZeroMQ publisher worker, per spec §5.1
// Runs on Core 6, dontwait send, SNDHWM=10000.
class PubWorker {
 public:
  explicit PubWorker(const std::string& endpoint);

  PubWorker(const PubWorker&) = delete;
  PubWorker& operator=(const PubWorker&) = delete;

  // Publish a tick. Uses dontwait — drops message if queue full.
  void PublishTick(std::shared_ptr<TickData> tick);

  uint64_t dropped_count() const { return dropped_count_; }

 private:
  zmq::context_t ctx_;
  zmq::socket_t pub_socket_;
  uint64_t dropped_count_ = 0;
};

}  // namespace sqc
