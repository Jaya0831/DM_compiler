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
    // struct representation
    // reference: https://mlir.llvm.org/docs/Dialects/LLVM/#structure-types
    struct struct_repr {
      const char* name;  // name of the struct (if applicable)
      size_t alignment;  // struct alignment in bytes
      size_t field_num;  // number of fields in the struct

      // fields
      struct struct_field {
        const char* name;   // field name (if applicable)
        struct type* type;  // field type
        size_t offset;      // field offset within the struct
      }* fields;
    } struct_repr;

    // array representation
    // we do not consider array type in v1.0
    struct array_repr {
      struct type* type;
      size_t len;
    } array_repr;

    // for TYPE_POINTER
    struct type* pointee;
  };
};

// ...like this
extern struct type* INT_TYPE;
extern struct type* FLOAT_TYPE;
extern struct type* DOUBLE_TYPE;
extern struct type* UINT64_T_TYPE;

// returns type id (uint8_t)
int register_struct(struct compute_context* ctx, struct struct_repr layout);
int register_array(struct compute_context* ctx, struct array_repr layout);

#ifdef __cplusplus
}
#endif
#endif
