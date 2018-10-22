#pragma once

#include <memory>

class FramedPacketSending;
class ServerThread;
class ClientThread;

class ITransport {
  friend ServerThread;
  friend ClientThread;

public:
  virtual int32_t sendPacket(int32_t fd, std::shared_ptr<uint8_t> packet,
                             uint32_t packentLen) = 0;

protected:
  virtual int32_t readFromSocket(int32_t fd) = 0;
  virtual int32_t sendToSocket(int32_t fd) = 0;
  virtual int32_t newConnection(int32_t fd, sockaddr_in &addr) = 0;
  virtual void connectionClosed(int32_t fd) = 0;

  virtual int32_t
  sendFramedPacket(int32_t fd, std::shared_ptr<FramedPacketSending> packet) = 0;
};
