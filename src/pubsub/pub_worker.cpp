#include "pub_worker.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

PubWorker::PubWorker(const std::string& endpoint) : ctx_(1), pub_socket_(ctx_, ZMQ_PUB) {
  pub_socket_.set(zmq::sockopt::sndhwm, 10000);
  pub_socket_.bind(endpoint);
  LOG_INFO(GetLogger(), "PubWorker bound to {}", endpoint);
}

void PubWorker::PublishTick(std::shared_ptr<TickData> tick) {
  if (!tick) return;

  // Copy 64 bytes into ZMQ message (safe: avoids shared_ptr lifetime issues)
  zmq::message_t msg(sizeof(TickData));
  std::memcpy(msg.data(), tick.get(), sizeof(TickData));

  zmq::send_result_t res = pub_socket_.send(std::move(msg), zmq::send_flags::dontwait);
  if (!res && errno == EAGAIN) {
    dropped_count_++;
  }
}

}  // namespace sqc
