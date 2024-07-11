#include "type.h"
#include "context-internal.h"

struct type* INT_TYPE = &(struct type){.kind = TYPE_OTHER, .size = sizeof(int)};
struct type* FLOAT_TYPE = &(struct type){.kind = TYPE_OTHER, .size = sizeof(float)};
struct type* DOUBLE_TYPE = &(struct type){.kind = TYPE_OTHER, .size = sizeof(double)};

struct type* UINT64_T_TYPE = &(struct type){.kind = TYPE_OTHER, .size = sizeof(uint64_t)};

// lzl: Only `register_type` is OK, since not all types will be directly allocated on the heap?
//
// Some types that are not `malloc`ed directly can be made oblivious, marking as TYPE_OTHER and
// storing only its size. These do not need its own cache space, thus will not have type ID.

int register_struct(struct compute_context* ctx, struct struct_repr layout) {
  // TODO
  return 0;
}

int register_array(struct compute_context* ctx, struct array_repr layout) {
  // TODO
  return 0;
}
