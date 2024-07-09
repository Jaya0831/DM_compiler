#include <stdint.h>
#include <stdlib.h>

#include "main.h"

int chunk_alloc(struct memory_context* ctx, uint16_t type_size, uint16_t max_count) {
  // TODO: this should be synchronised
  struct chunk* new_chunks =
    try_p(realloc(ctx->chunks, (ctx->chunk_count + 1) * sizeof(struct chunk)));
  ctx->chunks = new_chunks;

  // TODO: mmap large chunks
  uint8_t* buf = try_p(malloc(type_size * max_count));

  ctx->chunks[ctx->chunk_count] = (struct chunk){type_size, max_count, buf};
  return ctx->chunk_count++;
}

int chunk_free(int chunk_idx) {
  return 0;
}

// TODO: register memory region using RDMA
