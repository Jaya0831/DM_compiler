#ifndef __GLOBALALLOCATOR_H__
#define __GLOBALALLOCATOR_H__

#include <stdlib.h>
#include <stdint.h>
#include "Type.h"
#include "GlobalAddress.h"

#ifdef __cplusplus
extern "C"
{
#endif

GlobalAddress disaggAlloc(uint8_t typeID, size_t size, int count){}
void disaggFree(GlobalAddress gaddr, size_t size){}


#ifdef __cplusplus
}
#endif

#endif 