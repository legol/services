#pragma once

#include "framed_transport.h"
#include <cstring>
#include <sys/socket.h>
#include <mutex>

class FramedPacketSending : public FramedPacket {
  friend FramedTransport;

public:
  FramedPacketSending(std::shared_ptr<uint8_t> _body, uint32_t bodyLen);

protected:
  void reset();

  bool headerSent();
  bool allSent();

  uint8_t* remainingPointer();
  uint32_t bytesToSend();

  int32_t updateBytesSent(uint32_t bytes_sent);

private:
  uint32_t bytesToSendFromHeader();
  uint32_t bytesToSendFromBody();

  uint8_t* remainingHeaderPointer();
  uint8_t* remainingBodyPointer();

  uint32_t header_bytes_sent;
  uint32_t body_bytes_sent;
};


class FramedPacketSendingQ {
  friend FramedTransport;

public:
  FramedPacketSendingQ();

protected:
  int32_t pushPacket(std::shared_ptr<FramedPacketSending> packet);
  void popPacket();
  std::shared_ptr<FramedPacketSending> topPacket();
  bool empty();

private:
  std::deque<std::shared_ptr<FramedPacketSending>> sending_buffer;
  std::recursive_mutex sending_buffer_mutex;
};
