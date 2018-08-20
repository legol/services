#include <iostream>
#include <thread>
#include <memory>

#include "framed_transport.h"
#include "server_thread.h"

int32_t onPacketReceived(std::shared_ptr<FramedPacket> packet) {
  return 0;
}

int32_t onConnected(int32_t fd, sockaddr_in &addr) {
  printf("%s:%s\n", __FILE__, __FUNCTION__);
  return 0;
}

void onDisconnected(int32_t fd) {
  printf("%s:%s\n", __FILE__, __FUNCTION__);
}

int main()
{
  std::shared_ptr<ITransport> framed_transport = FramedTransport::create(
      std::function<int32_t(std::shared_ptr<FramedPacket>)>(onPacketReceived),
      std::function<int32_t(int32_t fd, sockaddr_in & addr)>(onConnected),
      std::function<void(int32_t)>(onDisconnected));

  std::shared_ptr<ServerThread> server_thread =
      ServerThread::create("192.168.1.171", 22334, framed_transport);

  server_thread->startAndJoin();

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
