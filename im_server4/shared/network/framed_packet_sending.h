#pragma once

#include "framed_transport.h"
#include <cstring>
#include <sys/socket.h>

class FramedPacketSending : public FramedPacket {
public:
  FramedPacketSending();
  void reset();

  bool headerSent();
  bool allSent();

  uint32_t bytesToSendFromHeader();
  uint32_t bytesToSendFromBody();

  uint8_t* remainingHeaderPointer();
  uint8_t* remainingBodyPointer();

  int32_t updateBytesSent(uint32_t bytes_sent);

private:
  uint32_t header_bytes_sent;
  uint32_t body_bytes_sent;
};


class FramedPacketSendingQ {
public:
  FramedPacketSendingQ();

  int32_t pushPacket(std::shared_ptr<FramedPacketSending> packet);
  std::shared_ptr<FramedPacketSending> topPacket();
  bool empty();

private:
  std::deque<std::shared_ptr<FramedPacketSending>> sending_buffer;
  std::mutex sending_buffer_mutex;
};
