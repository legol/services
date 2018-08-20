#pragma once

#include <memory>
#include <string>
#include <thread>

#include "epoll_event.h"
#include "transport.h"

class ClientThread {
public:
  virtual ~ClientThread();

  static std::shared_ptr<ClientThread>
  create(std::string _server_ip, int32_t _server_port, std::shared_ptr<ITransport> trasport);
  int32_t startAndJoin(); // will start a thread to run clientLoop()

  void closeConnection();

private:
  ClientThread();

  static void setNonBlocking(int32_t fd);
  int32_t connectToServer();

  int32_t onRead();
  int32_t onWrite();
  int32_t onError();
  int32_t onConnected(int32_t fd, sockaddr_in& addr);
  void onDisconnected(int32_t fd);

private:
  std::string server_ip;
  int32_t server_port;

  int32_t fd;
  std::shared_ptr<EpollEvent> epoll_event;
  std::shared_ptr<ITransport> transport;
};
