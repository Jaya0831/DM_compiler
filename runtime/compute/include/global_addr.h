#ifndef _COMPUTE_GLOBAL_ADDR_H_
#define _COMPUTE_GLOBAL_ADDR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

union global_addr {
  struct {
    uint8_t cache_id : 8; // start from 3 (reserve cache_id 0 to 2)
    uint64_t offset : 56;
  };
  uint64_t val;
};

// data dependency
typedef union global_addr (*data_dep_t)(union global_addr gaddr);

struct addr_dep {
  uint8_t type1, type2;
  data_dep_t dep;
};

#ifdef __cplusplus
}
#endif

#endif
