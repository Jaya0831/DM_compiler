#include <assert.h>
#include <stdatomic.h>

#include "addr.h"
#include "cache.h"
#include "context-internal.h"
#include "hashmap.h"

struct cache_token cache_request(struct compute_context* ctx, global_addr_t gaddr) {
  const struct addr_trans_entry* entry =
    hashmap_get(ctx->addr_trans_table, &(struct addr_trans_entry){.gaddr = gaddr});
  if (entry) {
    struct cache_token token = {
      .tag = extract_tag(ctx, gaddr),
      .type_id = gaddr.type_id,
      .slot_index = entry->slot_index,
    };

    struct cache_slot_metadata* meta = &ctx->caches[token.type_id].metadata[token.slot_index];
    if (meta->tag == token.tag) {
      // This cache slot is still ours
      // TODO: RC + 1
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

// TODO: cache_token_free to do `RC -= 1`
