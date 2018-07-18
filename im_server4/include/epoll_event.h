#ifndef _EPOLL_EVENT_INCLUDED_ 
#define _EPOLL_EVENT_INCLUDED_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>

class EpollEvent
{
	public:
		virtual ~EpollEvent(void);
		std::shared_ptr<EpollEvent> create();

		void epollWaitLoop();
	private:
		EpollEvent(int32_t _epoll_fd);
		
		int32_t registerSocket(
			int32_t fd, 
			uint32_t event,
			std::function<int(int)> onRead,
			std::function<int(int)> onWrite,
			std::function<int(int)> onError);
		int32_t unregisterSocket(const int32_t fd);

	private:		
		int32_t	epoll_fd;
};


#endif