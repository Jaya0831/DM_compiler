#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

#include "addr.h"
#include "alloc.h"
#include "common/handshake.h"
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

  struct chunk_ref_list* chunks = &ctx->type_chunk_refs[type_id];
  pthread_rwlock_rdlock(&chunks->lock);

  // find chunk available for allocation; this currently has to be the last chunk in
  // `ctx->type_chunk_refs[type_id]`, or all chunks are filled.
  if (!chunks->head ||
      (chunks->tail && (chunks->tail->next_addr + real_size >
                        ((chunks->tail->next_addr & (page_size - 1)) + page_size)))) {
    pthread_rwlock_unlock(&chunks->lock);
    uint64_t addr = atomic_fetch_add(&ctx->next_chunk, page_size);
    chunk_ref_list_insert(chunks, addr);
    if (addr >= mem_upper_bound(ctx->rdma->mem)) {
      // remote memory pool is drained
      errno = ENOMEM;
      return GADDR_NULL;
    }
    pthread_rwlock_rdlock(&chunks->lock);
  }

  // Now, `tail` contains the available chunk. Shift `next_addr` to allocate.
  // TODO: atomic
  gaddr.offset = atomic_fetch_add(&chunks->tail->next_addr, real_size);

  pthread_rwlock_unlock(&chunks->lock);
  return gaddr;
}

void disagg_free(struct compute_context* ctx, global_addr_t gaddr) {
  // currently no-op
}
