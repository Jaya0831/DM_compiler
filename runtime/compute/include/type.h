#ifndef _COMPUTE_TYPE_H_
#define _COMPUTE_TYPE_H_

#include <stdlib.h>

#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

// we care about struct and array (and pointer?)
// other types could just be defined by global const...
enum type_kind {
  TYPE_ANY,  // for type id 0 to 2
  TYPE_STRUCT,
  TYPE_ARRAY,
  TYPE_POINTER,
  TYPE_OTHER,
};

struct type {
  enum type_kind kind;
  size_t size;
  // to handle Additional Parametric Types
  union {
    // for nested TYPE_STRUCT
    struct {
      struct struct_repr* struct_repr;
      size_t alignment;
    };
    // for TYPE_ARRAY
    struct array_repr* array_repr;
    // for TYPE_POINTER
    struct type* pointee;
  };
};

// ...like this
// extern const struct type* INT_TYPE;

// struct representation
// reference: https://mlir.llvm.org/docs/Dialects/LLVM/#structure-types
struct struct_repr {
  const char* name;           // name of the struct
  struct struct_elem* elems;  // array of elements
  size_t elem_num;            // number of elements in the struct
};

struct struct_elem {
  const char* name;   // name of the element (if applicable)
  struct type* type;  // type of the element
  size_t offset;      // offset of the element within the struct
};

// array representation
// we do not consider `LLVMArray` type in v1.0
struct array_repr {
  struct type* type;
  size_t len;
};

// returns type id (uint8_t)
int register_struct(struct compute_context* ctx, struct struct_repr layout);
int register_array(struct compute_context* ctx, struct array_repr layout);

#ifdef __cplusplus
}
#endif
#endif
