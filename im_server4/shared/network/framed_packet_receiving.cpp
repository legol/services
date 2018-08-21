#include "framed_packet_receiving.h"
#include <cstring>
#include <sys/socket.h>

FramedPacketReceiving::FramedPacketReceiving() { reset(); }

void FramedPacketReceiving::reset() {
  header_bytes_read = 0;
  body_bytes_read = 0;

  header = {0};
  body.reset();
}

bool FramedPacketReceiving::completed() {
  return (headerCompleted() && body_bytes_read == header.body_length);
}

int32_t FramedPacketReceiving::appendToHeader(uint8_t *buffer,
                                              uint32_t bytes_to_append) {
  if (bytes_to_append + header_bytes_read > sizeof(FramedPacketHeader)) {
    return -1;
  }

  memcpy((uint8_t *)(&header) + header_bytes_read, buffer, bytes_to_append);
  header_bytes_read += bytes_to_append;

  if (header_bytes_read == sizeof(FramedPacketHeader)) {
    body = std::shared_ptr<uint8_t>(new uint8_t[header.body_length]);
  }

  return 0;
}

int32_t FramedPacketReceiving::appendToBody(uint8_t *buffer,
                                            uint32_t bytes_to_append) {
  if (!headerCompleted()) {
    return -1;
  }

  if (bytes_to_append + body_bytes_read > header.body_length) {
    return -1;
  }

  memcpy(body.get() + body_bytes_read, buffer, bytes_to_append);
  body_bytes_read += bytes_to_append;

  return 0;
}

bool FramedPacketReceiving::headerCompleted() {
  return header_bytes_read == sizeof(FramedPacketHeader);
}

uint32_t FramedPacketReceiving::bytesToReadIntoHeader() {
  return sizeof(FramedPacketHeader) - header_bytes_read;
}

uint32_t FramedPacketReceiving::bytesToReadIntoBody() {
  return header.body_length - body_bytes_read;
}
