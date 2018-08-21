#include "framed_transport.h"
#include <cstring>
#include <sys/socket.h>

class FramedPacketReceiving : public FramedPacket {
public:
  FramedPacketReceiving();
  void reset();

  bool completed();
  bool headerCompleted();

  int32_t appendToHeader(uint8_t *buffer, uint32_t bytes_to_append);
  int32_t appendToBody(uint8_t *buffer, uint32_t bytes_to_append);


  uint32_t bytesToReadIntoHeader();
  uint32_t bytesToReadIntoBody();

private:
  uint32_t header_bytes_read;
  uint32_t body_bytes_read;
};
