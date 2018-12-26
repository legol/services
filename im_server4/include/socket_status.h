#pragma once

#include <map>
#include <memory>
#include <fcntl.h>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <deque>
#include <mutex>

class SocketStatus {
public:
  SocketStatus() {
    can_read = false;
    can_write = false;
  }

public:
  bool can_read;
  bool can_write;
};
