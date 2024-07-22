#include <stdint.h>
#include <assert.h>

#include "addr.h"
#include "context-internal.h"

// gaddr.offset = tag + slot_off
uint64_t extract_tag(struct compute_context* ctx, global_addr_t gaddr) {
  uint64_t page_size = ctx->rdma->mem.page_size;
  uint64_t page_off_mask = page_size - 1, page_mask = ~page_off_mask;
  size_t type_size = ctx->types[gaddr.type_id]->size;
  return (gaddr.offset & page_mask) + ((gaddr.offset & page_off_mask) / type_size * type_size);
}
