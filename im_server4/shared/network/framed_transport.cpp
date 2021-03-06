#include "framed_transport.h"
#include "framed_packet_receiving.h"
#include "framed_packet_sending.h"
#include <cstring>
#include <sys/socket.h>

std::shared_ptr<ITransport> FramedTransport::create(
    std::function<int32_t(int32_t fd, std::shared_ptr<FramedPacket>)> onPacketReceived,
    std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected,
    std::function<void(int32_t fd)> onDisconnected) {
  auto instance = std::shared_ptr<FramedTransport>(new FramedTransport());
  instance->onPacketReceived = onPacketReceived;
  instance->onConnected = onConnected;
  instance->onDisconnected = onDisconnected;

  return instance;
}

void FramedTransport::updateSocketReadStatus(int32_t fd, bool canRead) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);

  auto itr = socket_status.find(fd);
  if (itr != socket_status.end()) {
    itr->second.can_read = canRead;
  }
}

void FramedTransport::updateSocketWriteStatus(int32_t fd, bool canWrite) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);

  auto itr = socket_status.find(fd);
  if (itr != socket_status.end()) {
    itr->second.can_write = canWrite;
  }
}

bool FramedTransport::canRead(int32_t fd) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);
  return socket_status[fd].can_read;
}

bool FramedTransport::canWrite(int32_t fd) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);
  return socket_status[fd].can_write;
}


int32_t FramedTransport::readFromSocket(int32_t fd) {
  updateSocketReadStatus(fd, true);

  const uint32_t buffer_len = 1024 * 1024; // 1M bytes
  uint8_t buffer[buffer_len] = {0};

  while (true) {
    int32_t bytes_received = recv(fd, buffer, buffer_len, 0);
    if (bytes_received == -1) {
      if ((EAGAIN == errno) || (EINTR == errno)) {
        updateSocketReadStatus(fd, false);
        return 0; // retry later
      }
      printf("%s:%d:%s readFromSocket fatal\n", __FILE__, __LINE__, __FUNCTION__);
      return -1;
    } else if (bytes_received == 0) {
      updateSocketReadStatus(fd, false);
      return 0;
    }

    auto itrReceiving = receiving_buffer.find(fd);
    if (itrReceiving == receiving_buffer.end()) {
      receiving_buffer[fd] =
          std::shared_ptr<FramedPacketReceiving>(new FramedPacketReceiving());
      itrReceiving = receiving_buffer.find(fd);
    }

    bool full_packet_received = false;
    auto receiving_packet = itrReceiving->second;
    auto process_result = forgeFrames(fd, receiving_packet, (uint8_t *)buffer,
                                      bytes_received, full_packet_received);
    if (full_packet_received) {
      receiving_buffer.erase(itrReceiving);
    }
    if (process_result != 0) {
      return process_result;
    }
  }

  return 0;
}

int32_t FramedTransport::sendToSocket(int32_t fd) {
  updateSocketWriteStatus(fd, true);

  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);

  auto itrQ = sending_buffer.find(fd);
  if (itrQ == sending_buffer.end()) {
    return 0;
  }

  auto sending_q = itrQ->second;
  while (!(sending_q->empty())) {
    auto sending_packet = sending_q->topPacket();
    auto bytes_sent = send(fd, sending_packet->remainingPointer(),
                        sending_packet->bytesToSend(), 0);

    if (bytes_sent == -1) {
      if ((EAGAIN == errno) || (EINTR == errno)) {
        // do nothing, wait for EPOLLOUT
        updateSocketWriteStatus(fd, false);
        return 0;
      } else {
        // real error
        return 0; // TODO: close connection
      }
    } else {
      // sent something.
      sending_packet->updateBytesSent(bytes_sent);

      if (sending_packet->allSent()) {
        sending_q->popPacket();
      }
    }
  }

  sending_buffer.erase(itrQ);
  return 0;
}

int32_t FramedTransport::forgeFrames(
    int32_t fd,
    std::shared_ptr<FramedPacketReceiving> receiving_packet, uint8_t *received,
    uint32_t bytes_received, bool &full_packet_received) {
  full_packet_received = false;

  if (receiving_packet.get() == nullptr) {
    return -1;
  }

  uint32_t bytes_consumed = 0;
  while (bytes_consumed < bytes_received) {
    uint32_t bytes_left = bytes_received - bytes_consumed;
    if (!(receiving_packet->headerCompleted())) {
      // forge header
      uint32_t bytes_to_read_into_header =
          receiving_packet->bytesToReadIntoHeader();
      receiving_packet->appendToHeader(
          received + bytes_consumed,
          std::min(bytes_to_read_into_header, bytes_left));
      bytes_consumed += std::min(bytes_to_read_into_header, bytes_left);

      if (receiving_packet->headerCompleted()) {
        // alloc memory for body
        receiving_packet->allocBody();
      }
    } else {
      // forge body
      if (receiving_packet->completed()) {
        // a completed packet received
        auto received_packet =
            std::dynamic_pointer_cast<FramedPacket>(receiving_packet);
        auto process_result = onPacketReceived(fd, received_packet);
        full_packet_received = true;
        if (process_result != 0) {
          return process_result;
        }
        receiving_packet->reset();
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

  if (receiving_packet->completed()) {
    // a completed packet received
    auto received_packet =
        std::dynamic_pointer_cast<FramedPacket>(receiving_packet);
    auto process_result = onPacketReceived(fd, received_packet);
    full_packet_received = true;
    if (process_result != 0) {
      return process_result;
    }
    receiving_packet->reset();

    return 0;
  }

  full_packet_received = false;
  return 0;
}

int32_t FramedTransport::newConnection(int32_t fd, sockaddr_in &addr) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);
  socket_status[fd] = SocketStatus();

  return onConnected(fd, addr);
}

void FramedTransport::connectionClosed(int32_t fd) {
  std::scoped_lock<std::recursive_mutex> lock(socket_status_mutex);
  socket_status.erase(fd);

  onDisconnected(fd);
}

int32_t
FramedTransport::sendFramedPacket(int32_t fd,
                            std::shared_ptr<FramedPacketSending> packet) {
  std::scoped_lock<std::recursive_mutex> lock(sending_buffer_mutex);

  auto itrQ = sending_buffer.find(fd);
  if (itrQ == sending_buffer.end()) {
    sending_buffer[fd] =
        std::shared_ptr<FramedPacketSendingQ>(new FramedPacketSendingQ());
    itrQ = sending_buffer.find(fd);
  }

  itrQ->second->pushPacket(packet);

  if (canWrite(fd)) {
    return sendToSocket(fd);
  } else {
    return 0;
  }
}

int32_t FramedTransport::sendPacket(int32_t fd, std::shared_ptr<uint8_t> packet,
                           uint32_t packentLen) {
  auto framedPacket = std::make_shared<FramedPacketSending>(packet, packentLen);
  return sendFramedPacket(fd, framedPacket);
}
