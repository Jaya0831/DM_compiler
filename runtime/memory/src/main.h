#ifndef _MEMORY_MAIN_H_
#define _MEMORY_MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#include "../../common/rdma.h"

struct memory_context;

/* alloc.c */

struct chunk {
  uint16_t type_size;
  uint32_t max_count;
  uint8_t* buf;
  // TODO: struct ibv_mr*
};

int chunk_alloc(struct memory_context* ctx, uint16_t type_size, uint16_t max_count);
int chunk_free(int chunk_idx);

/* rdma.c */

struct rdma_server {
  struct rdma_event_channel* rdma_events;
  struct rdma_cm_id* listen_id;
  struct rdma_connection* conn;
};

struct rdma_server* rdma_server_create(struct sockaddr* addr);
int rdma_server_free(struct rdma_server* server);

/* context.c */

struct memory_context {
  struct rdma_server* rdma;
  struct chunk* chunks;
  int chunk_count;
};

struct memory_context* memory_context_create();
int memory_context_free(struct memory_context* ctx);

/* main.c */

struct arguments {
  bool verbose;
  // const char* bind;
};

#endif
