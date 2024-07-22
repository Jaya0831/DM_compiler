#ifndef _COMPUTE_CONTEXT_INTERNAL_H_
#define _COMPUTE_CONTEXT_INTERNAL_H_

#include <errno.h>
#include <hashmap.h>
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
    _Atomic uint64_t next_addr;  // chunk_start_addr = next_addr & (chunk_size - 1);
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

struct cache_free_list {
  // TODO: queue or B-tree
};

void cache_free_list_push(int slot);
int cache_free_list_pop();

struct cache_block {
  size_t slot_count;
  void* slots;
  struct cache_slot_metadata {
    uint64_t tag;         // gaddr.offset = tag + slot_off
    _Atomic bool dirty;   // Dirty bit
    _Atomic uint32_t rc;  // Reference count
    // TODO: merge atomic types into one big atomic uint64_t
  }* metadata;
  struct cache_free_list* free_list;
};

// Internal representation of compute context
struct compute_context {
  struct rdma_client* rdma;  // RDMA context, containing remote memory info

  // Type information
  uint8_t types_count;  // Total number of types (including reserved)
  struct type** types;  // Type information

  // Allocation
  struct chunk_ref_list*
    type_chunk_refs;            // Allocated chunks' address reference for types,
                                // probably should be linked list for future GC compatibility
  _Atomic uint64_t next_chunk;  // Next allocated chunk start address;
                                // only for naive chunk allocation algorithm

  // Caching
  struct cache_block* caches;
  struct hashmap* addr_trans_table;

  // TODO: addr dep, ...
};

struct addr_trans_entry {
  global_addr_t gaddr;
  int slot_index;
};

static inline uint64_t addr_trans_entry_hash(const void* item, uint64_t seed0, uint64_t seed1) {
  const struct addr_trans_entry* entry = (typeof(entry))item;
  return entry->gaddr.val;
}

static inline struct hashmap* addr_trans_table_new() {
  return hashmap_new(sizeof(struct addr_trans_entry), 0, 0, 0, addr_trans_entry_hash, NULL, NULL,
                     NULL);
}

#endif
