#include "gtest/gtest.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>

#include "framed_transport.h"
#include "server_thread.h"

using namespace std::placeholders;

class EchoServer {
public:
  EchoServer() {
    numPacketsReceived = 0;
  }

  int32_t onPacketReceived(int32_t fd, std::shared_ptr<FramedPacket> packet) {
    numPacketsReceived++;
    printf("%s:%d:%s num_received=%d\n", __FILE__, __LINE__, __FUNCTION__,
           numPacketsReceived);

    std::string payload(packet->body.get(),
                        packet->body.get() + packet->header.body_len);
    printf("header len:%d payload:%s\n", packet->header.body_len,
           payload.c_str());

    std::string echoPayload = payload;
    std::shared_ptr<uint8_t> echoPacket =
        std::shared_ptr<uint8_t>(new uint8_t[echoPayload.length()]);
    memcpy(echoPacket.get(), echoPayload.c_str(), echoPayload.length());
    transport_->sendPacket(fd, echoPacket, echoPayload.length());

    return 0;
  }

  int32_t onConnected(int32_t fd, sockaddr_in &addr) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
    return 0;
  }

  void onDisconnected(int32_t fd) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
  }

  void run() {
    transport_ = FramedTransport::create(
        std::bind(&EchoServer::onPacketReceived, this, _1, _2),
        std::bind(&EchoServer::onConnected, this, _1, _2),
        std::bind(&EchoServer::onDisconnected, this, _1));

    std::shared_ptr<ServerThread> serverThread =
        ServerThread::create("192.168.101.64", 22334, transport_);

    serverThread->startAndJoin();
  }


protected:
  std::shared_ptr<ITransport> transport_;

  int32_t numPacketsReceived;
};

int main(int argc, char **argv) {
  EchoServer server;
  server.run();
}
