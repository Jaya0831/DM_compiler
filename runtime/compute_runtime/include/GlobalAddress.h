#ifndef __GLOBALADDRESS_H__
#define __GLOBALADDRESS_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef union GlobalAddress{
    struct {
        uint64_t nodeID:8;
        uint64_t cacheID:8; // start from 3 for compatibility
        uint64_t lAddr:48;
    };
    uint64_t val;
}GlobalAddress;

#ifdef __cplusplus
}
#endif




#endif