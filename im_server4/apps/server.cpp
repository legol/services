#include <iostream>
#include <thread>
#include <memory>

#include "framed_transport.h"
#include "server_thread.h"

// Use functions as callback
// int32_t onPacketReceived(int32_t fd, std::shared_ptr<FramedPacket> packet) {
//   printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
//   return 0;
// }
//
// int32_t onConnected(int32_t fd, sockaddr_in &addr) {
//   printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
//   return 0;
// }
//
// void onDisconnected(int32_t fd) {
//   printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
// }

// Use class method as callback:
class TestServer {
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
    return 0;
  }

  void onDisconnected(int32_t fd) {
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
  }
};

using namespace std::placeholders;

int main()
{
  // Use functions as callback
  // std::shared_ptr<ITransport> framed_transport = FramedTransport::create(
  //     std::function<int32_t(std::shared_ptr<FramedPacket>)>(onPacketReceived),
  //     std::function<int32_t(int32_t fd, sockaddr_in & addr)>(onConnected),
  //     std::function<void(int32_t)>(onDisconnected));

  // Use class method as callback
  TestServer server;
  std::shared_ptr<ITransport> framedTransport = FramedTransport::create(
      std::bind(&TestServer::onPacketReceived, &server, _1, _2),
      std::bind(&TestServer::onConnected, &server, _1, _2),
      std::bind(&TestServer::onDisconnected, &server, _1));


  std::shared_ptr<ServerThread> serverThread =
      ServerThread::create("192.168.101.116", 22334, framedTransport);

  serverThread->startAndJoin();

  // int x = 9;
  // std::thread threadObj([] {
  //   for (int i = 0; i < 10000; i++)
  //     std::cout << "Display Thread Executing" << std::endl;
  // });
  //
  // for (int i = 0; i < 10000; i++)
  //   std::cout << "Display From Main Thread" << std::endl;
  //
  // threadObj.join();
  // std::cout << "Exiting from Main Thread" << std::endl;
  return 0;
}
