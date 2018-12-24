#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <string.h>

#include "framed_transport.h"
#include "client_thread.h"

// Use functions as callback:
// int32_t onPacketReceived(int32_t fd, std::shared_ptr<FramedPacket> packet) {
//   return 0;
// }
//
// int32_t onConnected(int32_t fd, sockaddr_in &addr) {
//   printf("%s:%s\n", __FILE__, __FUNCTION__);
//   return 0;
// }
//
// void onDisconnected(int32_t fd) {
//   printf("%s:%s\n", __FILE__, __FUNCTION__);
// }

// Use class method as callback:
class TestClient {
public:
  int32_t onPacketReceived(int32_t fd, std::shared_ptr<FramedPacket> packet) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);

    std::string payload(packet->body.get(),
                        packet->body.get() + packet->header.body_len);
    printf("header len:%d payload:%s\n", packet->header.body_len,
           payload.c_str());

    return 0;
  }

  int32_t onConnected(int32_t fd, sockaddr_in &addr) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);

    // Send 2 packets

    std::string testPayload1 = "abcdefg";
    std::shared_ptr<uint8_t> packet1 =
        std::shared_ptr<uint8_t>(new uint8_t[testPayload1.length()]);
    memcpy(packet1.get(), testPayload1.c_str(), testPayload1.length());
    transport_->sendPacket(fd, packet1, testPayload1.length());


    std::string testPayload2 = "chenjie love cxf";
    std::shared_ptr<uint8_t> packet2 =
        std::shared_ptr<uint8_t>(new uint8_t[testPayload2.length()]);
    memcpy(packet2.get(), testPayload2.c_str(), testPayload2.length());
    transport_->sendPacket(fd, packet2, testPayload2.length());

    return 0;
  }

  void onDisconnected(int32_t fd) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
  }

  void setTransport(std::shared_ptr<ITransport> transport) {
    transport_ = transport;
  }

protected:
  std::shared_ptr<ITransport> transport_;
};

using namespace std::placeholders;

int main() {
  // Use functions as callback:
  // std::shared_ptr<ITransport> framed_transport = FramedTransport::create(
  //     std::function<int32_t(std::shared_ptr<FramedPacket>)>(onPacketReceived),
  //     std::function<int32_t(int32_t fd, sockaddr_in &addr)>(onConnected),
  //     std::function<void(int32_t)>(onDisconnected));

  // Use class method as callback:
  TestClient client;
  std::shared_ptr<ITransport> framedTransport = FramedTransport::create(
      std::bind(&TestClient::onPacketReceived, &client, _1, _2),
      std::bind(&TestClient::onConnected, &client, _1, _2),
      std::bind(&TestClient::onDisconnected, &client, _1));

  client.setTransport(framedTransport);

  std::shared_ptr<ClientThread> clientThread =
      ClientThread::create("192.168.101.64", 22334, framedTransport);

  clientThread->startAndJoin();

  return 0;
}
