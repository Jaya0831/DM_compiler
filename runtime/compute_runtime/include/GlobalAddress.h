#ifndef __GLOBALADDRESS_H__
#define __GLOBALADDRESS_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef union {
    struct {
        uint64_t cacheID:8; // start from 3 (reserve cacheID 0 to 2)
        uint64_t lAddr:56;
    };
    uint64_t val;
}GlobalAddress;

#ifdef __cplusplus
}
#endif




#endif