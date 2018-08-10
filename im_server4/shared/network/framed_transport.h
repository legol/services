#pragma once

#include <map>
#include <memory>

#include "transport.h"

#pragma pack(push, 1)
struct FramedPacketHeader {
  uint32_t body_length;
};
#pragma pack(pop)

class FramedPacket {
public:
  FramedPacketHeader header;
  std::shared_ptr<uint8_t> body;

  uint32_t header_bytes_read;
  uint32_t body_bytes_read;
};

class FramedTransport : public ITransport {
public:
  static std::shared_ptr<ITransport>
  create(std::function<int32_t(int32_t /*fd*/,
                               std::shared_ptr<FramedPacket> /*packet*/)>
             onPacketReceived);

  virtual int32_t readSocket(int32_t fd) override;

private:
  std::map<int32_t, FramedPacket> receiving_buffer;
  std::function<int32_t(int32_t /*fd*/,
                        std::shared_ptr<FramedPacket> /*packet*/)>
      packet_received_cb;
};
