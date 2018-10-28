#include "gtest/gtest.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>

#include "framed_transport.h"
#include "server_thread.h"

using namespace std::placeholders;

class TestServer {
public:
  int32_t onPacketReceived(std::shared_ptr<FramedPacket> packet) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);

    std::string payload(packet->body.get(),
                        packet->body.get() + packet->header.body_len);
    printf("header len:%d payload:%s\n", packet->header.body_len,
           payload.c_str());
    return 0;
  }

  int32_t onConnected(int32_t fd, sockaddr_in &addr) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
  }

  void onDisconnected(int32_t fd) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
  }
};

TEST(TestServer, testConnect) {
  TestServer server;
  std::shared_ptr<ITransport> framedTransport = FramedTransport::create(
      std::bind(&TestServer::onPacketReceived, &server, _1),
      std::bind(&TestServer::onConnected, &server, _1, _2),
      std::bind(&TestServer::onDisconnected, &server, _1));


  std::shared_ptr<ServerThread> serverThread =
      ServerThread::create("192.168.101.116", 22334, framedTransport);

  serverThread->startAndJoin();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
