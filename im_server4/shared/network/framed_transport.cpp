#include "framed_transport.h"
#include <cstring>
#include <sys/socket.h>

class FramedPacketInternal : public FramedPacket {
public:
  FramedPacketInternal() { reset(); }

  void reset() {
    header_bytes_read = 0;
    body_bytes_read = 0;

    header = {0};
    body.reset();
  }

  bool completed() {
    return (header_bytes_read == sizeof(FramedPacketHeader) &&
            body_bytes_read == header.body_length);
  }

  int32_t appendToHeader(uint8_t *buffer, uint32_t bytes_to_append) {
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

  int32_t appendToBody(uint8_t *buffer, uint32_t bytes_to_append) {
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

  bool headerCompleted() {
    return header_bytes_read == sizeof(FramedPacketHeader);
  }

  uint32_t bytesToReadIntoHeader() {
    return sizeof(FramedPacketHeader) - header_bytes_read;
  }

  uint32_t bytesToReadIntoBody() {
    return header.body_length - body_bytes_read;
  }

private:
  uint32_t header_bytes_read;
  uint32_t body_bytes_read;
};

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ITransport> FramedTransport::create(
    std::function<int32_t(std::shared_ptr<FramedPacket>)> onPacketReceived,
    std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected,
    std::function<void(int32_t fd)> onDisconnected) {
  auto instance = std::shared_ptr<FramedTransport>(new FramedTransport());
  instance->onPacketReceived = onPacketReceived;
  instance->onConnected = onConnected;
  instance->onDisconnected = onDisconnected;

  return instance;
}

int32_t FramedTransport::readSocket(int32_t fd) {
  const uint32_t buffer_len = 1024 * 1024; // 1M bytes
  uint8_t buffer[buffer_len] = {0};

  while (true) {
    auto bytes_received = recv(fd, buffer, buffer_len, 0);
    if (bytes_received == -1) {
      if ((EAGAIN == errno) || (EINTR == errno)) {
        return 0; // retry later
      }
      return -1;
    } else if (bytes_received == 0) {
      return -1;
    }

    auto itrReceiving = receiving_buffer.find(fd);
    if (itrReceiving == receiving_buffer.end()) {
      receiving_buffer[fd] =
          std::shared_ptr<FramedPacketInternal>(new FramedPacketInternal());
      receiving_buffer.find(fd);
    }

    auto receiving_packet = itrReceiving->second;
    auto process_result =
        forgeFrames(receiving_packet, (uint8_t *)buffer, bytes_received);
    if (process_result != 0) {
      return process_result;
    }
  }

  return 0;
}

int32_t FramedTransport::forgeFrames(
    std::shared_ptr<FramedPacketInternal> receiving_packet, uint8_t *received,
    uint32_t bytes_received) {
  if (receiving_packet.get() == nullptr) {
    return -1;
  }

  uint32_t bytes_consumed = 0;
  while (bytes_consumed < bytes_received) {
    auto bytes_left = bytes_received - bytes_consumed;
    if (!(receiving_packet->headerCompleted())) {
      // forge header
      uint32_t bytes_to_read_into_header =
          receiving_packet->bytesToReadIntoHeader();
      receiving_packet->appendToHeader(
          received + bytes_consumed,
          std::min(bytes_to_read_into_header, bytes_left));
      bytes_consumed += std::min(bytes_to_read_into_header, bytes_left);
    } else {
      // forge body
      if (receiving_packet->completed()) {
        // a completed packet received
        auto received_packet =
            std::dynamic_pointer_cast<FramedPacket>(receiving_packet);
        auto process_result = onPacketReceived(received_packet);
        if (process_result != 0) {
          return process_result;
        }
        receiving_packet.reset();
        continue;
      }

      uint32_t bytes_to_read_into_body =
          receiving_packet->bytesToReadIntoBody();
      receiving_packet->appendToBody(
          received + bytes_consumed,
          std::min(bytes_to_read_into_body, bytes_left));
      bytes_consumed += std::min(bytes_to_read_into_body, bytes_left);
    }
  }

  return 0;
}

int32_t FramedTransport::newConnection(int32_t fd, sockaddr_in &addr) {
  return onConnected(fd, addr);
}

void FramedTransport::connectionClosed(int32_t fd) {
  onDisconnected(fd);
}
