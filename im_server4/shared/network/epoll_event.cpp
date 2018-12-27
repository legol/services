#include "epoll_event.h"

#define MAX_NUM_EPOLL_EVENTS 1024
#define EPOLL_WAIT_TIMEOUT_MS 50

EpollEvent::EpollEvent(int32_t _epoll_fd) : epoll_fd(_epoll_fd) {
  terminate_ = false;
}

EpollEvent::~EpollEvent(void) {}

std::shared_ptr<EpollEvent> EpollEvent::create() {
  auto _epoll_fd = epoll_create(MAX_NUM_EPOLL_EVENTS * 2);
  if (_epoll_fd == -1) {
    return std::shared_ptr<EpollEvent>(NULL);
  }
  return std::shared_ptr<EpollEvent>(new EpollEvent(_epoll_fd));
}

int32_t EpollEvent::registerSocket(int32_t fd, uint32_t event,
                                   std::function<int32_t(int32_t)> onRead,
                                   std::function<int32_t(int32_t)> onWrite,
                                   std::function<int32_t(int32_t)> onError) {
  std::lock_guard<std::mutex> lock(callbacks_mutex);

  if (epoll_fd <= 0) {
    return -1;
  }

  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = event;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) != 0) {
    return -1;
  }

  callbacks[fd] = std::shared_ptr<EpollCallbacks>(
      new EpollCallbacks(onRead, onWrite, onError));
  return 0;
}

int32_t EpollEvent::unregisterSocket(int32_t fd) {
  std::lock_guard<std::mutex> lock(callbacks_mutex);

  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

  callbacks.erase(fd);
  return 0;
}

void EpollEvent::epollWaitLoop() {
  if (epoll_fd <= 0) {
    return;
  }

  ::epoll_event events[MAX_NUM_EPOLL_EVENTS];

  while (!terminate_) {
    // pEpollEventCallback->OnEpollOut( curr_fd, this ); // itrCallback->second
    // might be freed here, so I retain it as pEpollEventCallback.

    auto wait_ret =
        epoll_wait(epoll_fd, events, sizeof(events) / sizeof(events[0]),
                   EPOLL_WAIT_TIMEOUT_MS);
    if (wait_ret == -1) {
      int32_t e = errno;
      if (e == EINTR)
        continue;

      // OnError(fatal);
      printf("%s:%d:%s fatal\n", __FILE__, __LINE__, __FUNCTION__);
      break;
    }

    for (int32_t i = 0; i < wait_ret; ++i) {
      auto curr_fd = events[i].data.fd;
      auto curr_event = events[i].events;

      auto itrCallback = callbacks.find(curr_fd);
      if (itrCallback == callbacks.end()) {
        // OnError(not fatal);
        continue;
      }

      // itrCallback might be invalid after onXXXX()
      auto callback = itrCallback->second;
      if (curr_event & (EPOLLERR | EPOLLHUP)) {
        callback->onError(curr_fd);
      }

      if (curr_event & EPOLLIN) {
        callback->onRead(curr_fd);
      }

      if (curr_event & EPOLLOUT) {
        callback->onWrite(curr_fd);
      }
    }
  }
}

void EpollEvent::terminate() {
  terminate_ = true;
}
