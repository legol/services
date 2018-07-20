#include <thread>
#include <string>

class ServerThread {
	public:
		virtual ~ServerThread();

		int32_t create(std::string ip, int32_t port);
		int32_t start(); // will start a thread to run serverLoop()

	private:
		ServerThread();
		int32_t serverLoop();
};

