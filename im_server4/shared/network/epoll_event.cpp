#include "epoll_event.h"

#define MAX_NUM_EPOLL_EVENTS 1024
#define EPOLL_WAIT_TIMEOUT_MS 50

EpollEvent::EpollEvent(int32_t _epoll_fd): epoll_fd(_epoll_fd) {
}

EpollEvent::~EpollEvent(void) {
}

std::shared_ptr<EpollEvent> EpollEvent::create() {
    auto _epoll_fd = epoll_create(MAX_NUM_EPOLL_EVENTS * 2);
    if (_epoll_fd == -1)
    {
        return std::shared_ptr<EpollEvent>(NULL);
    }
	return std::shared_ptr<EpollEvent>(new EpollEvent(_epoll_fd));
}

int32_t EpollEvent::registerSocket(
		int32_t fd, 
		uint32_t event,
		std::function<int(int)> onRead,
		std::function<int(int)> onWrite,
		std::function<int(int)> onError) {
	return 0;
}

int32_t EpollEvent::unregisterSocket(int32_t fd) {
	return 0;
}

void EpollEvent::epollWaitLoop() {
	if (epoll_fd <= 0) {
		return;
	}

	::epoll_event events[MAX_NUM_EPOLL_EVENTS];

	while (true) {
		//pEpollEventCallback->OnEpollOut( curr_fd, this ); // itrCallback->second might be freed here, so I retain it as pEpollEventCallback.
		
		auto wait_ret = epoll_wait(epoll_fd, events, sizeof(events)/sizeof(events[0]), EPOLL_WAIT_TIMEOUT_MS);
		if (wait_ret == -1)
		{
			int32_t e = errno;
			if (e == EINTR)
				continue;

			//OnError();
			break;
		}

		for (int32_t i = 0 ; i < wait_ret; ++i)
		{
			auto curr_fd = events[i].data.fd;
			auto curr_event = events[i].events;

			if (curr_event & (EPOLLERR | EPOLLHUP))
			{
				//pEpollEventCallback->OnEpollErr( curr_fd, this );
			} 

			if (curr_event & EPOLLIN)
			{
				//pEpollEventCallback->OnEpollIn( curr_fd, this );
			}

			if (curr_event & EPOLLOUT)
			{
				//pEpollEventCallback->OnEpollOut( curr_fd, this ); // itrCallback->second might be freed here, so I retain it as pEpollEventCallback.
			}
		}
	}
}
