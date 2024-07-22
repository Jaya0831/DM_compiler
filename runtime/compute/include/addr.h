#ifndef _COMPUTE_GLOBAL_ADDR_H_
#define _COMPUTE_GLOBAL_ADDR_H_

#include <stdint.h>
#include <stdbool.h>

#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: pointer swizzling?
typedef union {
  struct {
    uint8_t type_id : 8;  // start from 3 (reserve type_id 0 to 2)
    uint64_t offset : 56;
  };
  uint64_t val;
} global_addr_t;

#define GADDR_NULL ((global_addr_t){.val = 0})

uint64_t extract_tag(struct compute_context* ctx, global_addr_t gaddr);

// data dependency
// TODO: register?
typedef global_addr_t (*const data_dep_t)(void* gaddr);

struct addr_dep {
  uint8_t from, to;
  data_dep_t dep;
};

#ifdef __cplusplus
}
#endif

#endif
