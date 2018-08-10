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
};

class FramedPacketInternal;

class FramedTransport : public ITransport {
public:
  static std::shared_ptr<ITransport> create(
      std::function<int32_t(std::shared_ptr<FramedPacket>)> onPacketReceived);

  virtual int32_t readSocket(int32_t fd) override;

private:
  int32_t forgeFrames(std::shared_ptr<FramedPacketInternal> receiving_packet,
                      uint8_t *received, uint32_t bytes_received);

private:
  std::map<int32_t, std::shared_ptr<FramedPacketInternal>> receiving_buffer;
  std::function<int32_t(std::shared_ptr<FramedPacket>)> onPacketReceived;
};
