#ifndef _COMPUTE_CACHE_H_
#define _COMPUTE_CACHE_H_

#include "addr.h"
#include "context-internal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cache_token {
  uint64_t tag;
  uint8_t type_id;
  int slot_index;
  int slot_off;
};

struct cache_token cache_request(struct compute_context* ctx, global_addr_t gaddr);
void* cache_access(struct compute_context* ctx, struct cache_token token);
void* cache_access_mut(struct compute_context* ctx, struct cache_token token);

#ifdef __cplusplus
}
#endif

#endif
