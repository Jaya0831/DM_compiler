#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "context-internal.h"
#include "type.h"

struct type INT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(int)};
struct type FLOAT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(float)};
struct type DOUBLE_TYPE = {.kind = TYPE_OTHER, .size = sizeof(double)};

struct type UINT64_T_TYPE = {.kind = TYPE_OTHER, .size = sizeof(uint64_t)};

int register_type(struct compute_context* ctx, struct type* type) {
  if (ctx->types_count >= 0xff) {
    fprintf(stderr, "type array full\n");
    return -1;
  }
  int idx = ctx->types_count++;
  ctx->types = try_p(reallocarray(ctx->types, ctx->types_count, sizeof(void*)));
  ctx->type_chunk_refs =
    try_p(reallocarray(ctx->type_chunk_refs, ctx->types_count, sizeof(struct chunk_ref_list)));
  pthread_rwlock_init(&ctx->type_chunk_refs[idx].lock, NULL);
  ctx->types[idx] = type;

  // TODO: initialize cache for this type

  return idx;
}
