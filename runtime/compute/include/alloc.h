#ifndef _COMPUTE_GLOBAL_ALLOC_H_
#define _COMPUTE_GLOBAL_ALLOC_H_

#include <stdlib.h>

#include "addr.h"

#ifdef __cplusplus
extern "C" {
#endif

global_addr_t disagg_alloc(uint8_t type_id, size_t size, int count);
void disagg_free(global_addr_t gaddr);

#ifdef __cplusplus
}
#endif

#endif
