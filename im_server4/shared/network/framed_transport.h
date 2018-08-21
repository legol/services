#pragma once

#include <map>
#include <memory>
#include <fcntl.h>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <deque>

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

class FramedPacketReceiving;
class FramedPacketSending;

class FramedTransport : public ITransport {
public:
  static std::shared_ptr<ITransport>
  create(std::function<int32_t(std::shared_ptr<FramedPacket>)> onPacketReceived,
         std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected,
         std::function<void(int32_t fd)> onDisconnected);

  virtual int32_t readFromSocket(int32_t fd) override; // called on epoll_in
  virtual int32_t sendToSocket(int32_t fd) override; // called on epoll_out

  virtual int32_t newConnection(int32_t fd, sockaddr_in &addr) override;
  virtual void connectionClosed(int32_t fd) override;

  virtual int32_t sendPacket(int32_t fd,
                             shared_ptr<FramedPacketSending> packet) override;

private:
  int32_t forgeFrames(std::shared_ptr<FramedPacketReceiving> receiving_packet,
                      uint8_t *received, uint32_t bytes_received,
                      bool &full_packet_received);

private:
  std::map<int32_t, std::shared_ptr<FramedPacketReceiving>> receiving_buffer;
  std::map<int32_t, std::shared_ptr<FramedPacketSendingQ>> sending_buffer;

  std::function<int32_t(std::shared_ptr<FramedPacket>)> onPacketReceived;
  std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected;
  std::function<void(int32_t fd)> onDisconnected;

};
