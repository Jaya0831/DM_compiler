#ifndef _COMPUTE_CONTEXT_INTERNAL_H_
#define _COMPUTE_CONTEXT_INTERNAL_H_

#include <errno.h>
#include <hashmap.h>
#include <infiniband/verbs.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "addr.h"
#include "rdma.h"
#include "type.h"

struct chunk_ref_list {
  pthread_rwlock_t lock;  // per-type chunk refs lock, used when adding new chunk
  struct chunk_ref {
    struct chunk_ref* next;
    uint64_t next_addr;  // chunk_start_addr = next_addr & (chunk_size - 1);
  } *head, *tail;
};

static inline int chunk_ref_list_insert(struct chunk_ref_list* list, uint64_t next_addr) {
  if (!list) return -(errno = EINVAL);
  struct chunk_ref* ref = try_p(calloc(1, sizeof(*ref)));

  pthread_rwlock_wrlock(&list->lock);
  ref->next_addr = next_addr;
  if (list->head) {
    list->tail = list->tail->next = ref;
  } else {
    list->head = list->tail = ref;
  }
  pthread_rwlock_unlock(&list->lock);
  return 0;
}

// TODO: chunk_ref_list_free

// Internal representation of compute context
struct compute_context {
  struct rdma_client* rdma;  // RDMA context, containing remote memory info

  // Type information
  uint8_t types_count;  // Total number of types (including reserved)
  struct type** types;  // Type information

  // Allocation
  struct chunk_ref_list* type_chunk_refs;  // Allocated chunks' address reference for types,
  _Atomic uint64_t next_chunk;             // Next allocated chunk start address;
                                           // only for naive chunk allocation algorithm

  // Caching
  struct cache_block* caches;
  struct hashmap* addr_trans_table;  // TODO: make it concurrent
                                     // probably create a C binding for Rust's dashmap?

  // TODO: addr dep, ...
};

struct addr_trans_entry {
  global_addr_t type_id_tag;
  int slot_index;
};

static inline uint64_t addr_trans_entry_hash(const void* item, uint64_t seed0, uint64_t seed1) {
  const struct addr_trans_entry* entry = (typeof(entry))item;
  return entry->type_id_tag.val;
}

static inline struct hashmap* addr_trans_table_new() {
  return hashmap_new(sizeof(struct addr_trans_entry), 0, 0, 0, addr_trans_entry_hash, NULL, NULL,
                     NULL);
}

#endif
