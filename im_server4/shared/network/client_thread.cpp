#include "client_thread.h"
#include <fcntl.h>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

using namespace std::placeholders; // for _1, _2, _3...

ClientThread::ClientThread() {}

ClientThread::~ClientThread() {}

std::shared_ptr<ClientThread>
ClientThread::create(std::string _server_ip, int32_t _server_port,
                     std::shared_ptr<ITransport> transport) {
  auto instance = std::shared_ptr<ClientThread>(new ClientThread());
  if (instance.get() == nullptr) {
    return std::shared_ptr<ClientThread>(nullptr);
  }

  instance->transport = transport;

  instance->epoll_event = EpollEvent::create();
  if (instance->epoll_event.get() == nullptr) {
    return std::shared_ptr<ClientThread>(nullptr);
  }

  instance->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (instance->fd == -1) {
    return std::shared_ptr<ClientThread>(nullptr);
  }

  instance->server_ip = _server_ip;
  instance->server_port = _server_port;

  return instance;
}

void ClientThread::setNonBlocking(int32_t fd) {
  int32_t flag = fcntl(fd, F_GETFL, 0);
  flag |= O_NONBLOCK;
  flag |= O_NDELAY;
  fcntl(fd, F_SETFL, flag);

  return;
}

int32_t ClientThread::connectToServer() {
  sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  inet_aton(server_ip.c_str(), (in_addr *)(&(addr.sin_addr.s_addr)));
  addr.sin_port = htons(server_port);

  int conn_r = ::connect(fd, (sockaddr *)&addr, sizeof(addr));
  int e = errno;
  if (-1 == conn_r) {
    if (e != EINPROGRESS) {
      return -1;
    } else {
      // do nothing
      printf("inprogress\n");
    }
  } else {
    if (onConnected(fd, addr) != 0) {
      return -1;
    }
  }

  return 0;
}

int32_t ClientThread::start() {
  if (connectToServer() != 0) {
      return -1;
  }

  ClientThread::setNonBlocking(fd);
  if (epoll_event->registerSocket(
          fd, EPOLLIN | EPOLLOUT | EPOLLET,
          std::bind(&ClientThread::onRead, this),
          std::bind(&ClientThread::onWrite, this),
          std::bind(&ClientThread::onError, this)) != 0) {
    return -1;
  }

  thread_ = std::make_shared<std::thread>(
      std::thread(std::bind(&EpollEvent::epollWaitLoop, epoll_event.get())));

  return 0;
}


int32_t ClientThread::startAndJoin() {
  start();
  thread_->join();
  return 0;
}

int32_t ClientThread::onRead() {
  if (transport->readFromSocket(fd) != 0) {
    closeConnection();
    return -1;
  }

  return 0;
}

int32_t ClientThread::onWrite() {
  if (transport->sendToSocket(fd) != 0) {
    closeConnection();
    return -1;
  }

  return 0;
}

int32_t ClientThread::onError() { return 0; }

int32_t ClientThread::onConnected(int32_t fd, sockaddr_in &addr) {
  return transport->newConnection(fd, addr);
}

void ClientThread::onDisconnected(int32_t fd) {
  transport->connectionClosed(fd);
}

void ClientThread::closeConnection() {
  onDisconnected(fd);
  epoll_event->unregisterSocket(fd);
  close(fd);
}

void ClientThread::terminate() {
  epoll_event->terminate();

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);

  thread_->join();
}
