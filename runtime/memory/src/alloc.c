#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "alloc.h"

static chunk_t* chunks = NULL;
static int chunk_count = 0;

int chunk_create(uint16_t type_size, uint16_t max_count) {
  // TODO: this should be synchronised
  chunks = realloc(chunks, (chunk_count + 1) * sizeof(chunk_t));
  if (!chunks) {
    return -errno;
  }

  // TODO: mmap large chunks
  uint8_t* buf = malloc(type_size * max_count);
  if (!buf) {
    return -errno;
  }

  chunks[chunk_count] = (chunk_t){type_size, max_count, buf};
  return chunk_count++;
}

// TODO: register memory region using RDMA
