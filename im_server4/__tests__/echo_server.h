#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>

#include "framed_transport.h"
#include "server_thread.h"

class EchoServer {
public:
  EchoServer();

  int32_t onPacketReceived(int32_t fd, std::shared_ptr<FramedPacket> packet);
  int32_t onConnected(int32_t fd, sockaddr_in &addr);
  void onDisconnected(int32_t fd);

  void run();
  void stop();

protected:
  std::shared_ptr<ITransport> transport_;
  std::shared_ptr<ServerThread> serverThread_;

  int32_t numPacketsReceived;
};
