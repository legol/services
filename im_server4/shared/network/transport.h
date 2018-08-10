#pragma once

class ITransport {
public:
  virtual int32_t readSocket(int32_t fd);
};
