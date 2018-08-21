#pragma once

class ITransport {
public:
  virtual int32_t readFromSocket(int32_t fd) = 0;
  virtual int32_t sendToSocket(int32_t fd) = 0;
  virtual int32_t newConnection(int32_t fd, sockaddr_in &addr) = 0;
  virtual void connectionClosed(int32_t fd) = 0;
};
