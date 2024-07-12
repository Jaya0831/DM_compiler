#include <stdint.h>
#include <stdio.h>

#include "addr.h"
#include "alloc.h"
#include "context-internal.h"

global_addr_t disagg_alloc(struct compute_context* ctx, uint8_t type_id, size_t size, int count) {
  global_addr_t gaddr = {.type_id = type_id};

  size_t real_size = type_id < 3 ? size : ctx->types[type_id]->size;
  uint64_t page_size = ctx->rdma->mem.page_size;
  if (real_size > page_size) {
    // Cross-page allocation not supported for now
    errno = ENOMEM;
    return (typeof(gaddr)){.val = 0};
  }

  // find chunk available for allocation; this currently has to be the last chunk in
  // `ctx->type_chunk_refs[type_id]`, or all chunks are filled.
  if (!ctx->type_chunk_refs[type_id].head) {
    chunk_ref_list_insert(&ctx->type_chunk_refs[type_id], ctx->next_chunk);
    ctx->next_chunk += page_size;
    // TODO: check if all memory pages have been allocated
  } else {
    uint64_t chunk_next_addr = ctx->type_chunk_refs[type_id].tail->next_addr;
    if (chunk_next_addr + real_size > ((chunk_next_addr & (page_size - 1)) + page_size)) {
      chunk_ref_list_insert(&ctx->type_chunk_refs[type_id], ctx->next_chunk);
      ctx->next_chunk += page_size;
      // TODO: check if all memory pages have been allocated
    }
  }

  // Now, `tail` contains the available chunk. Shift `next_addr` to allocate.
  uint64_t* next_addr_ptr = &ctx->type_chunk_refs[type_id].tail->next_addr;
  gaddr.offset = *next_addr_ptr;
  *next_addr_ptr += real_size;

  return gaddr;
}

void disagg_free(struct compute_context* ctx, global_addr_t gaddr) {
  // currently no-op
}
