#pragma once

#include <string>

#include <zmq.hpp>

namespace sqc {

// Unified gateway router, per spec §5.2
class GatewayRouter {
 public:
  explicit GatewayRouter(const std::string& bind_addr);
  ~GatewayRouter();

  void Run();

 private:
  zmq::context_t ctx_;
  zmq::socket_t xpub_;
  zmq::socket_t xsub_;
};

}  // namespace sqc
