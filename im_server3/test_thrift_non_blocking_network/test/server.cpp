
#include "Something.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TNonblockingServerSocket.h>


#include <memory>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::Test;
using namespace std;

class SomethingHandler : virtual public SomethingIf {
 public:
  SomethingHandler() {
    // Your initialization goes here
  }

  int32_t ping() {
    // Your implementation goes here
    printf("ping\n");
  }

};

int main(int argc, char **argv) {
	shared_ptr<SomethingHandler> handler(new SomethingHandler());
	shared_ptr<TProcessor> processor(new SomethingProcessor(handler));
  	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	
	shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(15);
	shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());

	threadManager->threadFactory(threadFactory);
	threadManager->start();

	shared_ptr<TNonblockingServerTransport>  serverTransport(new TNonblockingServerSocket(9090));
	TNonblockingServer server(processor, protocolFactory, serverTransport, threadManager);
  	server.serve();

	return 0;
}
