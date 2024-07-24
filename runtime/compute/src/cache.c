#include <assert.h>
#include <infiniband/verbs.h>
#include <stdatomic.h>

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

// Public APIs

struct cache_token cache_request(struct compute_context* ctx, global_addr_t gaddr) {
  uint64_t tag = extract_tag(ctx, gaddr);

  global_addr_t type_id_token = {.type_id = gaddr.type_id, .offset = tag};
  const struct addr_trans_entry* entry =
    hashmap_get(ctx->addr_trans_table, &(struct addr_trans_entry){type_id_token});
  if (entry) {
    struct cache_token token = {
      .tag = tag,
      .type_id = gaddr.type_id,
      .slot_index = entry->slot_index,
    };

    struct cache_slot_metadata* meta = &ctx->caches[token.type_id].metadata[token.slot_index];
    if (meta->tag == token.tag) {
      // This cache slot is still ours
      atomic_fetch_add(&meta->rc, 1);
      token.slot_off = gaddr.offset - token.tag;
      return token;
    }
  }

  // TODO: remote_fetch with possible eviction
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
  atomic_fetch_add(&meta->rc, -1);
}
