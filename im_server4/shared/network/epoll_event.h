#ifndef _EPOLL_EVENT_INCLUDED_ 
#define _EPOLL_EVENT_INCLUDED_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

class EpollCallbacks {
	public:
		EpollCallbacks(
			std::function<int32_t(int32_t)> _onRead,
			std::function<int32_t(int32_t)> _onWrite,
			std::function<int32_t(int32_t)> _onError
		): onRead(_onRead), onWrite(_onWrite), onError(_onError) {
		};

	public:
		std::function<int32_t(int32_t)>	onRead;
		std::function<int32_t(int32_t)>	onWrite;
		std::function<int32_t(int32_t)>	onError;
};

class EpollEvent {
	public:
		virtual ~EpollEvent(void);
		static std::shared_ptr<EpollEvent> create();
		int32_t registerSocket(
			int32_t fd, 
			uint32_t event,
			std::function<int32_t(int32_t)> onRead,
			std::function<int32_t(int32_t)> onWrite,
			std::function<int32_t(int32_t)> onError);
		int32_t unregisterSocket(const int32_t fd);

		void epollWaitLoop();
	private:
		EpollEvent(int32_t _epoll_fd);
		

	private:		
		int32_t	epoll_fd;
		std::unordered_map<int32_t, std::shared_ptr<EpollCallbacks>> callbacks;
		std::mutex callbacks_mutex;
};


#endif
