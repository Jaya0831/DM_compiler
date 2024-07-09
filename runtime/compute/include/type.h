#ifndef _COMPUTE_TYPE_H_
#define _COMPUTE_TYPE_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// types we care about
typedef enum {
  OBJ_STRUCT,
  // TODO:
} ObjectType;

typedef enum {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_DOUBLE,
  TYPE_POINTER,
  TYPE_ARRAY,
  TYPE_STRUCT,
  TYPE_UNKNOWN
  // TODO:
} Type;

struct llvm_struct;

struct element_type {
  Type type;
  size_t size;
  size_t alignment;

  // to handle Additional Parametric Types
  union {
    struct element_type *pointeeType;  // for POINTER type
    struct {
      struct element_type *elementType;
      size_t elementNum;
    } arrayInfo;             // for ARRAY type
    struct llvm_struct *structType;  // for nested STRUCT type
                             // TODO: other additional parametric types
  } info;
};

struct llvm_struct_element {
  struct element_type type;
  char *name;     // name of the element (if applicable)
  size_t offset;  // offset of the element within the struct
} LLVMStructElement;

struct llvm_struct {
  char *name;                   // name of the struct
  size_t elementNum;            // number of elements in the struct
  struct llvm_struct_element *elements;  // array of elements
};

// we do not consider `LLVMArray` type in v1.0
struct llvm_array {
  char *name;
  size_t elementNum;
  struct element_type *elementType;
};

// TODO: registerStuct & registerArray
int addLLVMStruct(uint8_t typeID, struct llvm_struct structLayout) {}
int addLLVMArray(uint8_t typeID, struct llvm_array arrayInfo) {}

#ifdef __cplusplus
}
#endif
#endif
