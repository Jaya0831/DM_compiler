#ifndef _COMPUTE_TYPE_H_
#define _COMPUTE_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

// If `registerType` is to be called right before the program, the LLVM
// `mlir::Type` must be converted to local C type.

typedef struct type {
  enum type_kind {
    TYPE_KIND_STRUCT,
    // TODO: other types
  } kind;
} type_t;

int register_type(type_t type);
int register_struct();

#ifdef __cplusplus
}
#endif
#endif
