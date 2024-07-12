#ifndef _COMPUTE_CONTEXT_INTERNAL_H_
#define _COMPUTE_CONTEXT_INTERNAL_H_

#include <errno.h>
#include <stdlib.h>

#include "rdma.h"
#include "type.h"

struct chunk_ref_list {
  struct chunk_ref {
    struct chunk_ref* next;
    uint64_t next_addr;  // chunk_start_addr = next_addr & (chunk_size - 1);
  } *head, *tail;
};

static inline int chunk_ref_list_insert(struct chunk_ref_list* list, uint64_t next_addr) {
  if (!list) return -(errno = EINVAL);
  struct chunk_ref* ref = try_p(calloc(1, sizeof(*ref)));
  ref->next_addr = next_addr;
  if (list->head) {
    list->tail = list->tail->next = ref;
  } else {
    list->head = list->tail = ref;
  }
  return 0;
}

// Internal representation of compute context
struct compute_context {
  struct rdma_client* rdma;  // RDMA context, containing remote memory info

  uint8_t types_count;  // Total number of types (including reserved)
  struct type** types;  // Type information
  struct chunk_ref_list*
    type_chunk_refs;    // Allocated chunks' address reference for types,
                        // probably should be linked list for future GC compatibility
  uint64_t next_chunk;  // Next allocated chunk start address;
                        // only for naive chunk allocation algorithm

  // TODO: cache, addr dep, ...
};

#endif
