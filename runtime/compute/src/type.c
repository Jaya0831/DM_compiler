#include <stdlib.h>

#include "context-internal.h"
#include "type.h"

struct type INT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(int)};
struct type FLOAT_TYPE = {.kind = TYPE_OTHER, .size = sizeof(float)};
struct type DOUBLE_TYPE = {.kind = TYPE_OTHER, .size = sizeof(double)};

struct type UINT64_T_TYPE = {.kind = TYPE_OTHER, .size = sizeof(uint64_t)};

int register_type(struct compute_context *ctx, struct type *type) {
  ctx->types = try_p(reallocarray(&ctx->types, ++ctx->types_count, sizeof(void *)));
  ctx->type_chunk_refs =
    try_p(reallocarray(&ctx->type_chunk_refs, ctx->types_count, sizeof(struct chunk_ref_list)));
  ctx->types[ctx->types_count - 1] = type;
  return ctx->types_count - 1;
}
