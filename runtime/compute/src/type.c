#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache-internal.h"
#include "common/try.h"
#include "context-internal.h"
#include "type.h"

struct type INT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(int)};
struct type FLOAT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(float)};
struct type DOUBLE_TYPE = {.kind = TYPE_OTHER, .size = sizeof(double)};

struct type UINT64_T_TYPE = {.kind = TYPE_OTHER, .size = sizeof(uint64_t)};

// TODO: properly do cleanup
int register_type(struct compute_context* ctx, struct type* type) {
  if (ctx->types_count >= 0xff) {
    fprintf(stderr, "type array full\n");
    return -1;
  }
  int idx = ctx->types_count++;

  // Allocate spaces for new types...
  ctx->types = try_p(reallocarray(ctx->types, ctx->types_count, sizeof(void*)));
  ctx->type_chunk_refs =
    try_p(reallocarray(ctx->type_chunk_refs, ctx->types_count, sizeof(struct chunk_ref_list)));
  ctx->caches = try_p(reallocarray(ctx->caches, ctx->types_count, sizeof(struct cache_block)));

  // ...and initialize
  ctx->types[idx] = type;
  try(pthread_rwlock_init(&ctx->type_chunk_refs[idx].lock, NULL), "failed to initialize rwlock");
  try(cache_block_init(ctx, idx, &ctx->caches[idx]), "failed to initialize cache block");

  return idx;
}
