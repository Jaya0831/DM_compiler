#ifndef _COMPUTE_GLOBAL_ADDR_H_
#define _COMPUTE_GLOBAL_ADDR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
  struct {
    uint8_t cache_id : 8; // start from 3 (reserve cache_id 0 to 2)
    uint64_t offset : 56;
  };
  uint64_t val;
} global_addr_t;

// data dependency
typedef global_addr_t (*const data_dep_t)(global_addr_t gaddr);

struct addr_dep {
  uint8_t type1, type2;
  data_dep_t dep;
};

#ifdef __cplusplus
}
#endif

#endif
