#ifndef _COMPUTE_CACHE_H_
#define _COMPUTE_CACHE_H_

#include "addr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cache_token {
  // TODO: define cache token
};

struct cache_token cache_request(global_addr_t gaddr);
void* cache_access(struct cache_token token);
void* cache_access_mut(struct cache_token token);

#ifdef __cplusplus
}
#endif

#endif
