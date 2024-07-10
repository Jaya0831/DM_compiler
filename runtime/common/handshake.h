// Information exchanged at creation of a connection.

#ifndef _COMMON_HANDSHAKE_H_
#define _COMMON_HANDSHAKE_H_

#include <stdint.h>

// Can't think of anything that compute side need to pass to memory side for now.
struct compute_info {
  uint8_t _reserved[16];
};

struct memory_info {
  uint64_t addr, page_size;
  uint32_t rkey, page_count;
};

#endif
