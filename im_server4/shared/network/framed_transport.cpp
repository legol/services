#include "framed_transport.h"
#include "framed_packet_receiving.h"
#include <cstring>
#include <sys/socket.h>

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

int32_t FramedTransport::readFromSocket(int32_t fd) {
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
          std::shared_ptr<FramedPacketReceiving>(new FramedPacketReceiving());
      receiving_buffer.find(fd);
    }

    bool full_packet_received = false;
    auto receiving_packet = itrReceiving->second;
    auto process_result = forgeFrames(receiving_packet, (uint8_t *)buffer,
                                      bytes_received, full_packet_received);
    if (full_packet_received) {
      receiving_buffer.remove(itrReceiving);
    }
    if (process_result != 0) {
      return process_result;
    }
  }

  return 0;
}

int32_t FramedTransport::sendToSocket(int32_t fd) {
  std::lock_guard<std::mutex> lock(sending_buffer_mutex);

  auto itrQ = sending_buffer.find(fd);
  if (itrQ == sending_buffer.end()) {
    return 0;
  }

  shared_ptr<FramedPacketSendingQ> sending_q = itrQ->second;

  while (!(sending_q->empty())) {
    auto sending_packet = sending_q->topPacket();

    int32_t bytes_sent = 0;
    if (!(sending_packet->headerSent())) {
      bytes_sent = send(fd, sending_packet->remainingHeaderPointer(),
                        sending_packet->bytesToSendFromHeader(), 0);
    } else {
      bytes_sent = send(fd, sending_packet->remainingBodyPointer(),
                        sending_packet->bytesToSendFromBody(), 0);
    }

    if (bytes_sent == -1) {
      if ((EAGAIN == errno) || (EINTR == errno)) {
        // do nothing
      } else {
        // real error
      }
    } else {
      // sent something.
      sending_packet->updateBytesSent(bytes_sent);

      if (sending_packet->allSent()) {
        sending_q->pop_front();
      }
    }
  }

  return 0;
}

int32_t FramedTransport::forgeFrames(
    std::shared_ptr<FramedPacketReceiving> receiving_packet, uint8_t *received,
    uint32_t bytes_received, bool &full_packet_received) {
  full_packet_received = false;

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
        full_packet_received = true;
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

int32_t FramedTransport::sendPacket(int32_t fd,
                                    shared_ptr<FramedPacketSending> packet) {
  auto itrQ = sending_buffer.find(fd);
  if (itrQ == sending_buffer.end()) {
    sending_buffer[fd] =
        std::make_shared<FramedPacketSendingQ>(new FramedPacketSendingQ());
    itrQ = sending_buffer.find(fd);
  }

  itrQ->second->pushPacket(packet);
  return 0;
}
