#ifndef _DM_LIB_H_
#define _DM_LIB_H_ 

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


// cache.h
typedef struct dm_cache_token {
  // TODO: define cache token
}dm_cache_token;

dm_cache_token dm_cache_request(dm_global_addr_t gaddr);
void* dm_cache_access(dm_cache_token token);
void* dm_cache_access_mut(dm_cache_token token);
// For multi-thread & dynamic Address Dependence
void dm_cache_store_notify(dm_cache_token token);
void dm_cache_load_notify(dm_cache_token token);


// global_addr.h
typedef union dm_global_addr_t{
  struct {
    uint8_t cache_id : 8; // start from 3 (reserve cache_id 0 to 2)
    uint64_t offset : 56;
  };
  uint64_t val;
}dm_global_addr_t;

typedef dm_global_addr_t (*dm_data_dep_t)(void* lptr);

typedef struct dm_addr_dep {
  uint8_t type1, type2;
  dm_data_dep_t dep;
}dm_addr_dep;

void dm_register_addr_dep(struct dm_addr_dep dep);


// global_alloc.h
dm_global_addr_t dm_malloc(uint8_t type_id);
void dm_free(dm_global_addr_t gaddr);


// type.h
struct dm_struct_repr;

typedef struct type {
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
    struct dm_struct_repr* repr;

    // TODO: other additional parametric types
  };
};

// reference: https://mlir.llvm.org/docs/Dialects/LLVM/#structure-types
typedef struct dm_struct_repr {
  const char* name;  // name of the struct
  struct struct_elem {
    const char* name;  // name of the element (if applicable)
    struct type type;  // type of the element
    size_t offset;     // offset of the element within the struct
  }* elems;            // array of elements
  size_t elem_num;     // number of elements in the struct
}dm_struct_repr;

uint8_t dm_register_struct(struct dm_struct_repr layout);



#ifdef __cplusplus
}
#endif

#endif
