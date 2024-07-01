#ifndef __CACHE_H_
#define __CACHE_H_

#include <stdint.h>
#include <stdlib.h>
#include "GlobalAddress.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct CacheToken {
    // TODO: define cache token
}CacheToken;

CacheToken cacheRequest(GlobalAddress gaddr){}
void* cacheAccess(GlobalAddress token){}
void* cacheAccessMut(GlobalAddress token){}

#ifdef __cplusplus
}
#endif

#endif