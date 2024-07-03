#ifndef _COMPUTE_GLOBAL_ALLOC_H_
#define _COMPUTE_GLOBAL_ALLOC_H_

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

void *disagg_alloc(type_t type, int count);

#ifdef __cplusplus
}
#endif

#endif
