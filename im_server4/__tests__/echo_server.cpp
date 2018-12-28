#include "echo_server.h"

using namespace std::placeholders;

EchoServer::EchoServer() {
  numPacketsReceived = 0;
}

int32_t EchoServer::onPacketReceived(int32_t fd,
                                     std::shared_ptr<FramedPacket> packet) {
  numPacketsReceived++;
  // printf("%s:%d:%s num_received=%d\n", __FILE__, __LINE__, __FUNCTION__,
  //        numPacketsReceived);

  std::string payload(packet->body.get(),
                      packet->body.get() + packet->header.body_len);
  // printf("header len:%d payload:%s\n", packet->header.body_len,
  //        payload.c_str());

  std::string echoPayload = payload;
  std::shared_ptr<uint8_t> echoPacket =
      std::shared_ptr<uint8_t>(new uint8_t[echoPayload.length()]);
  memcpy(echoPacket.get(), echoPayload.c_str(), echoPayload.length());
  transport_->sendPacket(fd, echoPacket, echoPayload.length());

  return 0;
}

int32_t EchoServer::onConnected(int32_t fd, sockaddr_in &addr) {
  printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
  return 0;
}

void EchoServer::onDisconnected(int32_t fd) {
  printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
}

void EchoServer::run() {
  transport_ = FramedTransport::create(
      std::bind(&EchoServer::onPacketReceived, this, _1, _2),
      std::bind(&EchoServer::onConnected, this, _1, _2),
      std::bind(&EchoServer::onDisconnected, this, _1));

  serverThread_ = ServerThread::create("192.168.101.64", 22334, transport_);
  serverThread_->start();
}

void EchoServer::stop() { serverThread_->terminate(); }
