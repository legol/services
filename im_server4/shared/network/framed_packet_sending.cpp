#include "framed_packet_sending.h"
#include <cstring>
#include <sys/socket.h>

FramedPacketSending::FramedPacketSending() {
  reset();
}

FramedPacketSending::reset() {
  header_bytes_sent = 0;
  body_bytes_sent = 0;
}

bool FramedPacketSending::headerSent() {
  return header_bytes_sent == sizeof(header);
}

bool FramedPacketSending::allSent() {
  return headerSent() && body_bytes_sent == header.body_length;
}

uint32_t FramedPacketSending::bytesToSendFromHeader() {
  return sizeof(FramedPacketHeader) - header_bytes_sent;
}

uint32_t FramedPacketSending::bytesToSendFromBody() {
  return header.body_length - body_bytes_sent;
}

uint8_t* FramedPacketSending::remainingHeaderPointer() {
  return ((uint8_t*)(&header)) + header_bytes_sent;
}

uint8_t* FramedPacketSending::remainingBodyPointer() {
  return body.get() + body_bytes_sent;
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
}


////////////////////////////////////////////////////////////////////////////////
FramedPacketSendingQ::FramedPacketSendingQ() {}

int32_t FramedPacketSendingQ::pushPacket(
    std::shared_ptr<FramedPacketSending> packet) {
  std::lock_guard<std::mutex> lock(sending_buffer_mutex);
  sending_buffer.push_back(packet);
}

bool FramedPacketSendingQ::empty() {
  std::lock_guard<std::mutex> lock(sending_buffer_mutex);
  return sending_buffer.empty();
}

std::shared_ptr<FramedPacketSending> FramedPacketSendingQ::topPacket() {
  std::lock_guard<std::mutex> lock(sending_buffer_mutex);

  if (sending_buffer.empty()) {
    return std::shared_ptr<FramedPacketSending>(nullptr);
  }

  return *(sending_buffer.begin());
}
