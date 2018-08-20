#include "server_thread.h"
#include <fcntl.h>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

using namespace std::placeholders; // for _1, _2, _3...

ServerThread::ServerThread() {}

ServerThread::~ServerThread() {}

std::shared_ptr<ServerThread>
ServerThread::create(std::string ip, int32_t port,
                     std::shared_ptr<ITransport> transport) {
  auto instance = std::shared_ptr<ServerThread>(new ServerThread());
  if (instance.get() == nullptr) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  instance->transport = transport;

  instance->epoll_event = EpollEvent::create();
  if (instance->epoll_event.get() == nullptr) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  instance->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (instance->listen_socket == -1) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  ServerThread::setNonBlocking(instance->listen_socket);

  int32_t reuse = 1;
  setsockopt(instance->listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse,
             sizeof(int));

  if (ServerThread::bindAddress(instance->listen_socket, ip, port) != 0) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  if (listen(instance->listen_socket, SOMAXCONN) == -1) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  if (instance->epoll_event->registerSocket(
          instance->listen_socket, EPOLLIN,
          std::bind(&ServerThread::onRead, instance.get(), _1),
          std::bind(&ServerThread::onWrite, instance.get(), _1),
          std::bind(&ServerThread::onError, instance.get(), _1)) != 0) {
    return std::shared_ptr<ServerThread>(nullptr);
  }

  return instance;
}

void ServerThread::setNonBlocking(int32_t fd) {
  int32_t flag = fcntl(fd, F_GETFL, 0);
  flag |= O_NONBLOCK;
  flag |= O_NDELAY;
  fcntl(fd, F_SETFL, flag);

  return;
}

int32_t ServerThread::bindAddress(int32_t fd, std::string ip, int32_t port) {
  sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  inet_aton(ip.c_str(), (in_addr *)(&(server_addr.sin_addr.s_addr)));
  server_addr.sin_port = htons(port);

  return bind(fd, (sockaddr *)&server_addr, sizeof(server_addr));
}

int32_t ServerThread::startAndJoin() {
  std::thread thread(std::bind(&EpollEvent::epollWaitLoop, epoll_event.get()));
  thread.join();

  return 0;
}

int32_t ServerThread::onRead(int32_t fd) {

  if (fd == listen_socket) {
    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int32_t new_socket = accept(fd, (sockaddr *)&addr, &addr_len);

    if (-1 == new_socket) {
      return 0;
    }

    if (onNewConnection(new_socket, addr) != 0) {
      closeConnection(new_socket);
    }
  } else {
    if (0 != transport->readSocket(fd)) {
      closeConnection(fd);
    }
  }
  return 0;
}

int32_t ServerThread::onWrite(int32_t fd) {
  // transport_write
  return 0;
}

int32_t ServerThread::onError(int32_t fd) { return 0; }

int32_t ServerThread::onNewConnection(int32_t fd, sockaddr_in &addr) {
  setNonBlocking(fd);
  if (epoll_event->registerSocket(
          fd, EPOLLIN | EPOLLOUT | EPOLLET,
          std::bind(&ServerThread::onRead, this, _1),
          std::bind(&ServerThread::onWrite, this, _1),
          std::bind(&ServerThread::onError, this, _1)) != 0) {
    closeConnection(fd);
  }

  return transport->newConnection(fd, addr);
}

void ServerThread::onConnectionClosed(int32_t fd) {
  transport->connectionClosed(fd);
}

void ServerThread::closeConnection(int32_t fd) {
  onConnectionClosed(fd);
  epoll_event->unregisterSocket(fd);
  close(fd);
}
