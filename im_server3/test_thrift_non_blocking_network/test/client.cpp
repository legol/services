
#include "Something.h"

#include <thrift/protocol/TBinaryProtocol.h>
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

int main(int argc, char **argv) {
    shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
    shared_ptr<TTransport> transport(new TFramedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    SomethingClient client(protocol);
    transport->open();
    client.ping();
	transport->close();

	return 0;
}
