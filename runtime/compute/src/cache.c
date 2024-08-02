#include <assert.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_verbs.h>
#include <stdatomic.h>
#include <stdint.h>

#include "addr.h"
#include "cache-internal.h"
#include "cache.h"
#include "common/try.h"
#include "context-internal.h"
#include "hashmap.h"

static const uint32_t FLAG_VALID = 1 << 31, FLAG_IO = 1 << 30;

void cache_block_free(struct cache_block* self) {
  if (self->free_list) cache_free_list_free(self->free_list);
  free(self->metadata);
  if (self->mr) ibv_dereg_mr(self->mr);
  free(self->slots);
}

int cache_block_init(struct compute_context* ctx, uint8_t type_id, struct cache_block* block) {
  // TODO: customize slot_count
  block->slot_count = 256;
  block->slots = try3_p(calloc(block->slot_count, ctx->types[type_id]->size));
  block->mr =
    try3_p(ibv_reg_mr(ctx->rdma->conn->pd, block->slots,
                      block->slot_count * ctx->types[type_id]->size, IBV_ACCESS_LOCAL_WRITE),
           "failed to register MR");
  // tags are already initialized to 0 by calloc
  block->metadata = try3_p(calloc(block->slot_count, sizeof(*block->metadata)));
  block->free_list = try3_p(cache_free_list_create(), "failed to create free list");
  return 0;
cleanup:
  cache_block_free(block);
  return -errno;
}

// Concurrent scenarios:
// * `addr_trans_table` contains `gaddr`:
//   - invalid: in eviction, cache miss
//   - valid: ours, cache hit
// * `addr_trans_table` does not contain `gaddr`: cache miss
static inline struct cache_token cache_try_request(struct compute_context* ctx, global_addr_t gaddr,
                                                   int free_slot) {
  uint64_t tag = extract_tag(ctx, gaddr);
  struct cache_block* cache = &ctx->caches[gaddr.type_id];

  struct addr_trans_entry key = {{.type_id = gaddr.type_id, .offset = tag}};
  const struct addr_trans_entry* entry;
  if (free_slot < 0) {
    entry = hashmap_get(ctx->addr_trans_table, &key);
  } else {
    // hashmap_get_or_insert mockup:
    //   void* hashmap_get_or_insert(struct hashmap* map, const void* item)
    //   return NULL if inserted, or the existing item inside the map
    // concurrent hash map implementation should have this method for atomicity
    entry = hashmap_get(ctx->addr_trans_table, &key);
    if (!entry) {
      struct addr_trans_entry key_value = key;
      key_value.slot_index = free_slot;
      assert(hashmap_set(ctx->addr_trans_table, &key_value) == NULL);
    }
  }

  if (entry) {
    struct cache_token token = {
      .tag = tag,
      .type_id = gaddr.type_id,
      .slot_index = entry->slot_index,
      .slot_off = gaddr.offset - tag,
    };

    struct cache_slot_metadata* meta = &cache->metadata[token.slot_index];
    uint32_t valid_io_rc = atomic_load(&meta->valid_io_rc);
    if ((valid_io_rc & FLAG_VALID) && meta->tag == tag) {
      // This cache slot is ours
      // if I/O bit set, spin until it is unset
      if (valid_io_rc & FLAG_IO)
        while (atomic_load(&meta->valid_io_rc) & FLAG_IO);
      atomic_fetch_add(&meta->valid_io_rc, 1);
      return token;
    }
  }
  return CACHE_TOKEN_NULL;
}

// Public APIs

struct cache_token cache_request(struct compute_context* ctx, global_addr_t gaddr) {
  struct cache_token token = cache_try_request(ctx, gaddr, -1);
  if (!cache_token_is_null(token)) return token;

  struct cache_block* cache = &ctx->caches[gaddr.type_id];
  int slot = cache_free_list_pop(cache->free_list);
  if (slot < 0) {
    // TODO: batch evict, evict delegation
    // TODO: initialize random seed
  retry:;
    int evict_idx = rand() % cache->slot_count;
    struct cache_slot_metadata* meta = &cache->metadata[evict_idx];
    // if valid, not in I/O and RC=0, mark it as invalid and evict
    uint32_t criteria = FLAG_VALID;
    if (atomic_compare_exchange_strong(&meta->valid_io_rc, &criteria, 0)) {
      struct addr_trans_entry entry_to_remove = {{.type_id = gaddr.type_id, .offset = meta->tag}};
      hashmap_delete(ctx->addr_trans_table, &entry_to_remove);
      if (atomic_exchange(&meta->dirty, false)) {
        size_t type_size = ctx->types[gaddr.type_id]->size;
        rdma_post_write(ctx->rdma->conn->id, NULL, cache->slots + evict_idx * type_size, type_size,
                        cache->mr, IBV_SEND_SIGNALED, meta->tag, ctx->rdma->mem.rkey);
        struct ibv_wc wc;
        rdma_get_send_comp(ctx->rdma->conn->id, &wc);
        // TODO: error handling
      }
      meta->tag = 0;
      slot = evict_idx;
    } else {
      goto retry;
    }
  }

  struct cache_slot_metadata* meta = &cache->metadata[slot];
  meta->tag = extract_tag(ctx, gaddr);
  meta->dirty = false;
  atomic_store(&meta->valid_io_rc, FLAG_VALID | FLAG_IO | 0);
  // memory fence here (to sync meta->tag)?

  token = cache_try_request(ctx, gaddr, slot);
  if (cache_token_is_null(token)) {
    size_t type_size = ctx->types[gaddr.type_id]->size;
    rdma_post_read(ctx->rdma->conn->id, NULL, cache->slots + slot * type_size, type_size, cache->mr,
                   IBV_SEND_SIGNALED, meta->tag, ctx->rdma->mem.rkey);
    struct ibv_wc wc;
    rdma_get_send_comp(ctx->rdma->conn->id, &wc);
    // TODO: error handling

    // fetch done, unset I/O bit and bump RC
    atomic_store(&meta->valid_io_rc, FLAG_VALID | 1);
    token = (typeof(token)){
      .tag = meta->tag,
      .type_id = gaddr.type_id,
      .slot_index = slot,
      .slot_off = gaddr.offset - meta->tag,
    };
  } else {
    cache_free_list_push(cache->free_list, slot);
  }

  return token;

  // -----------
  // TODO: remote_fetch with possible eviction
  // 1. check free list to see if there's free slots available; if not, evict to make one
  // 2. fill the acquired free slot with remote fetch
  // 3. add the entry to addr_trans_table
}

void* cache_access(struct compute_context* ctx, struct cache_token token) {
  void* ptr = ctx->caches[token.type_id].slots +
              token.slot_index * ctx->types[token.type_id]->size + token.slot_off;
  return ptr;
}

void* cache_access_mut(struct compute_context* ctx, struct cache_token token) {
  atomic_exchange(&ctx->caches[token.type_id].metadata[token.slot_index].dirty, true);
  return cache_access(ctx, token);
}

// TODO: prevent double free
void cache_token_free(struct compute_context* ctx, struct cache_token token) {
  struct cache_slot_metadata* meta = &ctx->caches[token.type_id].metadata[token.slot_index];
  assert(meta->tag == token.tag);
  atomic_fetch_add(&meta->valid_io_rc, -1);
}
