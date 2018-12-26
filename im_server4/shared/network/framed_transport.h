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
#include <mutex>

#include "transport.h"
#include "socket_status.h"

#pragma pack(push, 1)
struct FramedPacketHeader {
  uint32_t body_len;
};
#pragma pack(pop)

class FramedPacket {
public:
  FramedPacket() {
    header = {0};
  }

  FramedPacket(std::shared_ptr<uint8_t> _body, uint32_t bodyLen) {
    body = _body;
    header.body_len = bodyLen;
  }

  FramedPacketHeader header;
  std::shared_ptr<uint8_t> body;
};

class FramedPacketReceiving;
class FramedPacketSending;
class FramedPacketSendingQ;
class ServerThread;
class ClientThread;

class FramedTransport : public ITransport {
public:
  static std::shared_ptr<ITransport>
  create(std::function<int32_t(int32_t fd, std::shared_ptr<FramedPacket>)>
             onPacketReceived,
         std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected,
         std::function<void(int32_t fd)> onDisconnected);

  virtual int32_t sendPacket(int32_t fd, std::shared_ptr<uint8_t> packet,
                             uint32_t packentLen) override;

protected:
  virtual int32_t readFromSocket(int32_t fd) override; // called on epoll_in
  virtual int32_t sendToSocket(int32_t fd) override; // called on epoll_out

  virtual int32_t newConnection(int32_t fd, sockaddr_in &addr) override;
  virtual void connectionClosed(int32_t fd) override;

  virtual int32_t
  sendFramedPacket(int32_t fd,
                   std::shared_ptr<FramedPacketSending> packet) override;

private:
  int32_t forgeFrames(int32_t fd,
                      std::shared_ptr<FramedPacketReceiving> receiving_packet,
                      uint8_t *received, uint32_t bytes_received,
                      bool &full_packet_received);

  void updateSocketReadStatus(int32_t fd, bool canRead);
  void updateSocketWriteStatus(int32_t fd, bool canWrite);
  bool canRead(int32_t fd);
  bool canWrite(int32_t fd);

private:
  std::map<int32_t, std::shared_ptr<FramedPacketReceiving>> receiving_buffer;

  std::map<int32_t, std::shared_ptr<FramedPacketSendingQ>> sending_buffer;
  std::recursive_mutex sending_buffer_mutex;

  std::map<int32_t, SocketStatus> socket_status;
  std::recursive_mutex socket_status_mutex;

  std::function<int32_t(int32_t fd, std::shared_ptr<FramedPacket>)>
      onPacketReceived;
  std::function<int32_t(int32_t fd, sockaddr_in &addr)> onConnected;
  std::function<void(int32_t fd)> onDisconnected;
};
