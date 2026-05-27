#include "gateway_router.h"

#include "quill/LogMacros.h"
#include "common/logger_init.h"

namespace sqc {

GatewayRouter::GatewayRouter(const std::string& bind_addr) : ctx_(1) {
  xpub_ = zmq::socket_t(ctx_, ZMQ_XPUB);
  xsub_ = zmq::socket_t(ctx_, ZMQ_XSUB);

  xpub_.bind(bind_addr);
  xsub_.bind("ipc:///tmp/gateway_xsub.ipc");

  LOG_INFO(GetLogger(), "GatewayRouter started on {}", bind_addr);
}

GatewayRouter::~GatewayRouter() {
  xpub_.close();
  xsub_.close();
}

void GatewayRouter::Run() {
  // Standard ZMQ proxy: forward messages between XPUB and XSUB
  zmq::proxy(xsub_, xpub_);
}

}  // namespace sqc
