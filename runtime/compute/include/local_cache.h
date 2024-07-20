#ifndef LOCAL_CACHE_H_
#define LOCAL_CACHE_H_

#include <stddef.h>

#include "hashmap.h"

#ifdef __cplusplus
extern "C" {
#endif

struct local_cache {
  // Key(int): type ID
  // Value: type size (size_t), count of slots (size_t) and the local address of block (void *)
  struct hashmap *map_;
};

void local_cache_init(struct local_cache *self);
void *local_cache_new_block(struct local_cache *self, int type_id, size_t type_size, size_t count);
void *local_cache_get_block(struct local_cache *self, int type_id);
void *local_cache_get_random_slot(struct local_cache *self, int type_id);
void local_cache_destroy(struct local_cache *self);

#ifdef __cplusplus
}
#endif

#endif
