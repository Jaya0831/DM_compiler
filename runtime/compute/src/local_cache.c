#include "local_cache.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct local_cache_map_entry {
  int type_id;
  size_t type_size;
  size_t slot_count;
  void *local_addr;
};

uint64_t local_cache_map_entry_hash(const void *item, uint64_t seed0, uint64_t seed1) {
  const struct local_cache_map_entry *entry = item;
  return entry->type_id;
}

void local_cache_map_entry_elfree(void *item) {
  struct local_cache_map_entry *entry = item;

  free(entry->local_addr);
}

void local_cache_init(struct local_cache *self) {
  self->map_ = hashmap_new(sizeof(struct local_cache_map_entry), 0, 0, 0,
                           local_cache_map_entry_hash, NULL, local_cache_map_entry_elfree, NULL);
}

void *local_cache_new_block(struct local_cache *self, int type_id, size_t type_size, size_t count) {
  struct local_cache_map_entry *entry =
    (struct local_cache_map_entry *)hashmap_get(self->map_, &(struct local_cache_map_entry){
                                                              .type_id = type_id,
                                                            });
  if (entry) {
    entry->type_size = type_size;
    entry->slot_count = count;
    entry->local_addr = realloc(entry->local_addr, type_size * count);

    return entry->local_addr;
  }

  void *local_addr = malloc(type_size * count);

  hashmap_set(self->map_, &(struct local_cache_map_entry){
                            .type_id = type_id,
                            .type_size = type_size,
                            .slot_count = count,
                            .local_addr = local_addr,
                          });

  return local_addr;
}

void *local_cache_get_block(struct local_cache *self, int type_id) {
  const struct local_cache_map_entry *entry =
    hashmap_get(self->map_, &(struct local_cache_map_entry){
                              .type_id = type_id,
                            });

  if (!entry) return NULL;

  return entry->local_addr;
}

void *local_cache_get_random_slot(struct local_cache *self, int type_id) {
  const struct local_cache_map_entry *entry =
    hashmap_get(self->map_, &(struct local_cache_map_entry){
                              .type_id = type_id,
                            });

  if (!entry) return NULL;

  // FIXME: use uniformly distributed random
  ptrdiff_t index = rand() % entry->slot_count;
  return entry->local_addr + index;
}

void local_cache_destroy(struct local_cache *self) { hashmap_free(self->map_); }
