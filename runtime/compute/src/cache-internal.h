#ifndef _COMPUTE_CACHE_INTERNAL_H_
#define _COMPUTE_CACHE_INTERNAL_H_

#include "context-internal.h"

struct cache_free_list_node {
  int begin;
  int back;

  struct cache_free_list_node* next;
};

struct cache_free_list {
  struct cache_free_list_node* head;
  struct cache_free_list_node* tail;
};

struct cache_free_list* cache_free_list_new(int slot_begin, int slot_back);
void cache_free_list_push(struct cache_free_list* self, int slot);
// Returns free slot index, or negative value if no free slot is currently available.
int cache_free_list_pop(struct cache_free_list* self);
void cache_free_list_free(struct cache_free_list* self);

struct cache_block {
  size_t slot_count;
  void* slots;
  struct ibv_mr* mr;
  struct cache_slot_metadata {
    uint64_t tag;                  // gaddr.offset = tag + slot_off
    _Atomic bool dirty;            // Dirty bit
    _Atomic uint32_t valid_io_rc;  // [valid:1|io:1|rc:30]
  }* metadata;
  struct cache_free_list* free_list;
};

int cache_block_init(struct compute_context* ctx, uint8_t type_id, struct cache_block* block);
void cache_block_free(struct cache_block* self);

#endif
