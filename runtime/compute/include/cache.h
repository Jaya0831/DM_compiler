#ifndef _COMPUTE_CACHE_H_
#define _COMPUTE_CACHE_H_

#include "global_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cache_token {
  // TODO: define cache token
} cache_token_t;

cache_token_t cache_request(global_addr_t gaddr);
void *cache_access(global_addr_t token);
void *cache_access_mut(global_addr_t token);

#ifdef __cplusplus
}
#endif

#endif
