#ifndef _COMPUTE_GLOBAL_ADDR_H_
#define _COMPUTE_GLOBAL_ADDR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union global_addr {
  struct {
    uint8_t cache_id : 8; // start from 3 for compatibility
    uint64_t offset : 56;
  };
  uint64_t val;
} global_addr_t;

#ifdef __cplusplus
}
#endif

#endif
