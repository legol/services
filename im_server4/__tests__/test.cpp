#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>

#include "framed_transport.h"
#include "client_thread.h"

using namespace std::placeholders;

class MockClient {
 public:
  MOCK_METHOD2(onPacketReceived, int32_t(int32_t, std::shared_ptr<FramedPacket>));
  MOCK_METHOD2(onConnected, int32_t(int32_t, sockaddr_in&));
  MOCK_METHOD1(onDisconnected, int32_t(int32_t));
};

TEST(TestClient, testConnect) {
  MockClient client;
  std::shared_ptr<ITransport> framedTransport = FramedTransport::create(
      std::bind(&MockClient::onPacketReceived, &client, _1, _2),
      std::bind(&MockClient::onConnected, &client, _1, _2),
      std::bind(&MockClient::onDisconnected, &client, _1));

  std::shared_ptr<ClientThread> clientThread =
      ClientThread::create("192.168.101.64", 22334, framedTransport);

  EXPECT_CALL(client, onConnected(testing::_, testing::_)).Times(1);

  clientThread->connectToServer();
}

class MockClient1 {
 public:
  MOCK_METHOD2(onPacketReceived, int32_t(int32_t, std::shared_ptr<FramedPacket>));

  int32_t onConnected(int32_t fd, sockaddr_in &addr) {
    std::string testPayload1 = "abcdefg";
    std::shared_ptr<uint8_t> packet1 =
        std::shared_ptr<uint8_t>(new uint8_t[testPayload1.length()]);
    memcpy(packet1.get(), testPayload1.c_str(), testPayload1.length());
    transport_->sendPacket(fd, packet1, testPayload1.length());

    return 0;
  }

  MOCK_METHOD1(onDisconnected, int32_t(int32_t));

  void setTransport(std::shared_ptr<ITransport> transport) {
    transport_ = transport;
  }

protected:
  std::shared_ptr<ITransport> transport_;
};

TEST(TestClient, testSendPacket) {
  MockClient1 client;
  std::shared_ptr<ITransport> framedTransport = FramedTransport::create(
      std::bind(&MockClient1::onPacketReceived, &client, _1, _2),
      std::bind(&MockClient1::onConnected, &client, _1, _2),
      std::bind(&MockClient1::onDisconnected, &client, _1));

  client.setTransport(framedTransport);

  std::shared_ptr<ClientThread> clientThread =
      ClientThread::create("192.168.101.64", 22334, framedTransport);

  // We'll send a packet to echo server on connect
  // echo server will send it back so our onPacketReceived() should be called
  EXPECT_CALL(client, onPacketReceived(testing::_, testing::_)).Times(1);

  clientThread->start();

  // Do not exit immediately.
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);

  clientThread->terminate();
}

int main(int argc, char **argv) {
  // Run echo_server before running this test
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
