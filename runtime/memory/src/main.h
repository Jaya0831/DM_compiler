#ifndef _MEMORY_MAIN_H_
#define _MEMORY_MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#include "../../common/rdma.h"

/* rdma.c */

struct rdma_server {
  struct rdma_event_channel* rdma_events;
  struct rdma_cm_id* listen_id;
  struct rdma_connection* conn;
};

struct rdma_server* rdma_server_create(struct ibv_pd* pd, struct sockaddr* addr,
                                       void* handshake_data, size_t handshake_data_len);
int rdma_server_free(struct rdma_server* server);

/* context.c */

struct memory_context {
  struct rdma_server* rdma;
  struct ibv_mr* mem_pool;
};

struct memory_context* memory_context_create(uint16_t listen_port, size_t mem_pool_size);
int memory_context_free(struct memory_context* ctx);

/* main.c */

struct arguments {
  bool verbose;
  const char* port_str;
};

#endif
