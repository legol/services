#include "framed_packet_sending.h"
#include <cstring>
#include <sys/socket.h>

FramedPacketSending::FramedPacketSending(std::shared_ptr<uint8_t> _body,
                                         uint32_t bodyLen)
    : FramedPacket(_body, bodyLen) {
  reset();
}

void FramedPacketSending::reset() {
  header_bytes_sent = 0;
  body_bytes_sent = 0;
}

bool FramedPacketSending::headerSent() {
  return header_bytes_sent == sizeof(header);
}

bool FramedPacketSending::allSent() {
  return headerSent() && body_bytes_sent == header.body_len;
}

uint32_t FramedPacketSending::bytesToSendFromHeader() {
  return sizeof(FramedPacketHeader) - header_bytes_sent;
}

uint32_t FramedPacketSending::bytesToSendFromBody() {
  return header.body_len - body_bytes_sent;
}

uint32_t FramedPacketSending::bytesToSend() {
  if (!headerSent()) {
    return bytesToSendFromHeader();
  } else {
    return bytesToSendFromBody();
  }
}

uint8_t* FramedPacketSending::remainingHeaderPointer() {
  return ((uint8_t*)(&header)) + header_bytes_sent;
}

uint8_t* FramedPacketSending::remainingBodyPointer() {
  return body.get() + body_bytes_sent;
}

uint8_t* FramedPacketSending::remainingPointer() {
  if (!headerSent()) {
    return remainingHeaderPointer();
  } else {
    return remainingBodyPointer();
  }
}

int32_t FramedPacketSending::updateBytesSent(uint32_t bytes_sent) {
  while (bytes_sent != 0) {
    if (!headerSent()) {
      uint32_t bytes_into_header = std::min(bytes_sent, bytesToSendFromHeader());
      header_bytes_sent += bytes_into_header;
      bytes_sent -= bytes_into_header;
    } else {
      uint32_t bytes_into_body = std::min(bytes_sent, bytesToSendFromBody());
      body_bytes_sent += bytes_into_body;
      bytes_sent -= bytes_into_body;
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
FramedPacketSendingQ::FramedPacketSendingQ(){}

int32_t FramedPacketSendingQ::pushPacket(
    std::shared_ptr<FramedPacketSending> packet) {
  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);
  sending_buffer.push_back(packet);

  return 0;
}

void FramedPacketSendingQ::popPacket() {
  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);
  sending_buffer.pop_front();
}

bool FramedPacketSendingQ::empty() {
  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);
  return sending_buffer.empty();
}

std::shared_ptr<FramedPacketSending> FramedPacketSendingQ::topPacket() {
  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);

  if (sending_buffer.empty()) {
    return std::shared_ptr<FramedPacketSending>(nullptr);
  }

  return *(sending_buffer.begin());
}
