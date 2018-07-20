#include <functional>
#include <fcntl.h>
#include "server_thread.h"

using namespace std::placeholders;  // for _1, _2, _3...

ServerThread::ServerThread() {
}

ServerThread::~ServerThread() {
}

std::shared_ptr<ServerThread> ServerThread::create(std::string ip, int32_t port) {
	auto instance = std::shared_ptr<ServerThread>(new ServerThread());
	if (instance.get() == nullptr) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	instance->epoll_event = EpollEvent::create();
	if (instance->epoll_event.get() == nullptr) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	instance->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (instance->listen_socket == -1) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	if (ServerThread::setNonBlocking(instance->listen_socket) != 0) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	int32_t reuse = 1;
    setsockopt(instance->listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (int));

	if (ServerThread::bindAddress(instance->listen_socket, ip, port) != 0) {
		return std::shared_ptr<ServerThread>(nullptr);
	}
	
	if (listen(instance->listen_socket, SOMAXCONN) == -1) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	if (instance->epoll_event->registerSocket(
		instance->listen_socket, 
		EPOLLIN, 
		std::bind(&ServerThread::onRead, instance.get(), _1),
		std::bind(&ServerThread::onWrite, instance.get(), _1),
		std::bind(&ServerThread::onError, instance.get(), _1)) != 0) {
		return std::shared_ptr<ServerThread>(nullptr);
	}

	return instance;
}

int32_t ServerThread::setNonBlocking(int32_t fd)
{
    int32_t flag = fcntl(fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    flag |= O_NDELAY;
    fcntl(fd, F_SETFL, flag);
    return 0;
}

int32_t ServerThread::bindAddress(int32_t fd, std::string ip, int32_t port)
{
    sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    inet_aton(ip.c_str(), (in_addr*)(&(server_addr.sin_addr.s_addr)));
    server_addr.sin_port = htons(port);

    return bind(fd, (sockaddr*)&server_addr, sizeof(server_addr));
}

int32_t ServerThread::start() {
	std::thread thread(std::bind(&EpollEvent::epollWaitLoop, epoll_event.get()));
	thread.join();

	return 0;
}

int32_t ServerThread::onRead(int32_t fd) {

    if ( NULL == m_pEpollEvent.get() )
    {
        return;
    }

    if (fd == m_nListenFd)
    {
        // ÐÂÁ¬½Ó
        sockaddr_in addr;
        socklen_t addr_len = sizeof ( addr );
        int32_t   new_fd   = accept ( fd, (sockaddr*)&addr, &addr_len);

        if ( -1 == new_fd)
        {
            return;
        }

        if (m_pTcpEventCallback)
        {
            m_pTcpEventCallback->OnNewSocket( new_fd, this);
        }

        CNetworkUtil::SetNonBlockSocket( new_fd );
        if (-1 == m_pEpollEvent->RegisterEvent(new_fd, EPOLLIN | EPOLLOUT | EPOLLET, m_pCallback_IEpollEventCallback ) )
        {
            Close (fd);
        }

        if (m_pTcpEventCallback)
        {
            if (-1 == m_pTcpEventCallback->OnConnected (new_fd, addr, this))
            {
                Close (new_fd);
            }
        }
    }
    else
    {
        // Êý¾Ýµ½À´
        if (m_pTcpEventCallback)
        {
            if ( -1 == m_pTcpEventCallback->OnDataIn(fd, this) )
            {
                Close (fd);
            }
        }
    }
	return 0;
}

int32_t ServerThread::onWrite(int32_t fd) {
	return 0;
}

int32_t ServerThread::onError(int32_t fd) {
	return 0;
}


