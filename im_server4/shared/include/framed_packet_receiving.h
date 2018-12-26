#pragma once

#include "framed_transport.h"
#include <cstring>
#include <sys/socket.h>

class FramedPacketReceiving : public FramedPacket {
  friend FramedTransport;

public:
  FramedPacketReceiving();

protected:
  void reset();

  bool completed();
  bool headerCompleted();

  int32_t appendToHeader(uint8_t *buffer, uint32_t bytes_to_append);

  int32_t allocBody();
  int32_t appendToBody(uint8_t *buffer, uint32_t bytes_to_append);


  uint32_t bytesToReadIntoHeader();
  uint32_t bytesToReadIntoBody();

public:
  uint32_t header_bytes_read;
  uint32_t body_bytes_read;
};
