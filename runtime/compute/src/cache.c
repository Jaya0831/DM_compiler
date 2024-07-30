#include <assert.h>
#include <infiniband/verbs.h>
#include <stdatomic.h>
#include <stdint.h>

#include "addr.h"
#include "cache-internal.h"
#include "cache.h"
#include "common/try.h"
#include "context-internal.h"
#include "hashmap.h"

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

// - `addr_trans_table` contains `gaddr`:
//   - invalid: in eviction, cache miss
//   - valid: ours, cache hit
// - `addr_trans_table` does not contain `gaddr`: cache miss
static inline struct cache_token cache_try_request(struct compute_context* ctx,
                                                   global_addr_t gaddr) {
  uint64_t tag = extract_tag(ctx, gaddr);
  struct cache_block* cache = &ctx->caches[gaddr.type_id];

  global_addr_t type_id_tag = {.type_id = gaddr.type_id, .offset = tag};
  const struct addr_trans_entry* entry =
    hashmap_get(ctx->addr_trans_table, &(struct addr_trans_entry){type_id_tag});
  if (entry) {
    struct cache_token token = {
      .tag = tag,
      .type_id = gaddr.type_id,
      .slot_index = entry->slot_index,
      .slot_off = gaddr.offset - tag,
    };

    struct cache_slot_metadata* meta = &cache->metadata[token.slot_index];
    if ((atomic_load(&meta->valid_rc) & (1 << 31)) && meta->tag == tag) {
      // This cache slot is still ours
      // TODO: if I/O bit set, spin until it is unset
      atomic_fetch_add(&meta->valid_rc, 1);
      return token;
    }
  }
  return CACHE_TOKEN_NULL;
}

// Public APIs

struct cache_token cache_request(struct compute_context* ctx, global_addr_t gaddr) {
  struct cache_token token = cache_try_request(ctx, gaddr);
  if (!cache_token_is_null(token)) return token;

  struct cache_block* cache = &ctx->caches[gaddr.type_id];
  int slot = cache_free_list_pop(cache->free_list);
  if (slot < 0) {
    // TODO: batch evict, evict delegation
    // TODO: initialize random seed & random with exclusion
    int evict_idx = rand() % cache->slot_count;
    struct cache_slot_metadata* meta = &cache->metadata[evict_idx];
    // if valid and RC=0, mark it as invalid and evict
    uint32_t valid_rc = 1 << 31;
    if (atomic_compare_exchange_strong(&meta->valid_rc, &valid_rc, 0)) {
      hashmap_delete(
        ctx->addr_trans_table,
        &(struct addr_trans_entry){.type_id_tag = {.type_id = gaddr.type_id, .offset = meta->tag}});

      if (atomic_exchange(&meta->dirty, false)) {
        // TODO: get the global address and write back
      }
      meta->tag = 0;
      slot = evict_idx;
    } else {
      // TODO: try again
    }
  }

  // TODO: concurrent hash map implementation should have "insert if vacant" to ensure atomicity;
  // use a mockup for now
  // TODO: set I/O bit before hash map insertion
  token = cache_try_request(ctx, gaddr);
  if (cache_token_is_null(token)) {
    uint64_t tag = extract_tag(ctx, gaddr);
    struct addr_trans_entry entry = {
      .type_id_tag = {.type_id = gaddr.type_id, .offset = tag},
      .slot_index = slot,
    };
    hashmap_set(ctx->addr_trans_table, &entry);

    // TODO: mark the slot as valid and RC=1, insert to `addr_trans_table` and return token
    struct cache_slot_metadata* meta = &cache->metadata[slot];
    meta->tag = tag;
    meta->dirty = false;

    // TODO: RDMA fetch

    atomic_store(&meta->valid_rc, (1 << 31) + 1);

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
  atomic_fetch_add(&meta->valid_rc, -1);
}
