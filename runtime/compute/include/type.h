#ifndef _COMPUTE_TYPE_H_
#define _COMPUTE_TYPE_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct struct_repr;

struct type {
  // we care about struct and array (and pointer?)
  // lzl: if we only care about these, why don't we just make every other type `TYPE_OTHER` or
  // something?
  enum type_kind {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNKNOWN
    // TODO: other types
  } kind;
  size_t size;
  size_t alignment;

  // to handle Additional Parametric Types
  union {
    // for TYPE_POINTER
    struct type* pointee;

    // for TYPE_ARRAY
    struct {
      struct type* elem_type;
      size_t elem_num;
    };

    // for nested TYPE_STRUCT
    struct struct_repr* repr;

    // TODO: other additional parametric types
  };
};

// reference: https://mlir.llvm.org/docs/Dialects/LLVM/#structure-types
struct struct_repr {
  const char* name;  // name of the struct
  struct struct_elem {
    const char* name;  // name of the element (if applicable)
    struct type type;  // type of the element
    size_t offset;     // offset of the element within the struct
  }* elems;            // array of elements
  size_t elem_num;     // number of elements in the struct
};

// we do not consider `LLVMArray` type in v1.0
struct llvm_array {
  const char* name;
  struct type* type;
  size_t len;
};

int register_struct(uint8_t type_id, struct struct_repr layout);
int register_array(uint8_t type_id, struct llvm_array layout);

#ifdef __cplusplus
}
#endif
#endif
