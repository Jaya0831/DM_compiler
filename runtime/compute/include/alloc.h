#ifndef _COMPUTE_GLOBAL_ALLOC_H_
#define _COMPUTE_GLOBAL_ALLOC_H_

#include <stdint.h>
#include <stdlib.h>

#include "addr.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

global_addr_t disagg_alloc(struct compute_context* ctx, uint8_t type_id, size_t size, int count);
void disagg_free(struct compute_context* ctx, global_addr_t gaddr);

#ifdef __cplusplus
}
#endif

#endif
