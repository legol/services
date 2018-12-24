#pragma once

#include <memory>
#include <string>
#include <thread>

#include "epoll_event.h"
#include "transport.h"

class ServerThread {
public:
  virtual ~ServerThread();

  static std::shared_ptr<ServerThread>
  create(std::string ip, int32_t port, std::shared_ptr<ITransport> trasport);
  int32_t startAndJoin(); // will start a thread to run serverLoop()
  int32_t start(); // will start a thread to run clientLoop() but will not block current thread

  void terminate(); // gracefully exit

private:
  ServerThread();

  static void setNonBlocking(int32_t fd);
  static int32_t bindAddress(int32_t fd, std::string ip, int32_t port);

  int32_t onRead(int32_t fd);
  int32_t onWrite(int32_t fd);
  int32_t onError(int32_t fd);

  int32_t onNewConnection(int32_t fd, sockaddr_in& addr);
  void onConnectionClosed(int32_t fd);

  void closeConnection(int32_t fd);

private:
  int32_t listen_socket;
  std::shared_ptr<EpollEvent> epoll_event;
  std::shared_ptr<ITransport> transport;

  std::shared_ptr<std::thread> thread_;
};
