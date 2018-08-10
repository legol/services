#include "framed_transport.h"

std::shared_ptr<ITransport> FramedTransport::create(
    std::function<int32_t(int32_t /*fd*/,
                          std::shared_ptr<FramedPacket> /*packet*/)>
        onPacketReceived) {
  auto instance = std::shared_ptr<FramedTransport>(new FramedTransport());
  instance->packet_received_cb = onPacketReceived;

  return instance;
}

int32_t FramedTransport::readSocket(int32_t fd) {
  return 0;
}
