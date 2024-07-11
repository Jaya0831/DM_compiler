#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../common/try.h"
#include "context.h"
#include "type.h"

struct bin_tree_node {
  uint64_t val;
  struct bin_tree_node *left, *right;
};

int main() {
  struct type* bin_tree_node = calloc(1, sizeof(struct type));
  *bin_tree_node = (struct type){
    .kind = TYPE_STRUCT,
    .size = 24,
    .struct_repr.name = "struct bin_tree_node",
    .struct_repr.alignment = 8,
    .struct_repr.field_num = 3,
    .struct_repr.fields = calloc(3, sizeof(struct struct_field)),
  };

  struct type* bin_tree_node_ptr = calloc(1, sizeof(struct type));
  *bin_tree_node_ptr = (struct type){
    .kind = TYPE_POINTER,
    .size = 8,
    .pointee = bin_tree_node,
  };

  struct struct_field* fields = bin_tree_node->struct_repr.fields;
  fields[0] = (struct struct_field){.name = "val", .offset = 0, .type = UINT64_T_TYPE};
  fields[1] = (struct struct_field){.name = "left", .offset = 8, .type = bin_tree_node_ptr};
  fields[2] = (struct struct_field){.name = "right", .offset = 16, .type = bin_tree_node_ptr};

  struct compute_context* ctx = try_p(compute_context_create(), "failed to create compute context");
  fprintf(stderr, "compute context initialized\n");

  // TODO: register type

  while (1)
    ;

  try(compute_context_free(ctx), "failed to free");
  return 0;
}
