#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "common/try.h"
#include "context.h"
#include "type.h"

struct bin_tree_node {
  uint64_t val;
  struct bin_tree_node *left, *right;
};

struct type bin_tree_node = {
  .kind = TYPE_STRUCT,
  .size = sizeof(struct bin_tree_node),
  .struct_repr.name = "struct bin_tree_node",
  .struct_repr.alignment = alignof(struct bin_tree_node),
  .struct_repr.field_num = 3,
  .struct_repr.fields =
    (struct struct_field[]){
      {.name = "val", .offset = offsetof(struct bin_tree_node, val), .type = &UINT64_T_TYPE},
      {.name = "left", .offset = offsetof(struct bin_tree_node, left), .type = &bin_tree_node},
      {.name = "right", .offset = offsetof(struct bin_tree_node, right), .type = &bin_tree_node},
    },
};

uint8_t bin_tree_node_id = 0;

int main() {
  struct compute_context* ctx = try_p(compute_context_create(), "failed to create compute context");
  fprintf(stderr, "compute context initialized\n");
  bin_tree_node_id = try(register_type(ctx, &bin_tree_node));

  while (1)
    ;

  try(compute_context_free(ctx), "failed to free");
  return 0;
}
