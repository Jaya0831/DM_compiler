#ifndef _MEMORY_ALLOC_H_
#define _MEMORY_ALLOC_H_

#include <stdint.h>

typedef struct chunk {
  uint16_t type_size;
  uint32_t max_count;
  uint8_t* buf;
} chunk_t;

int chunk_create(uint16_t type_size, uint16_t max_count);

#endif
