#ifndef _COMPUTE_RDMA_H_
#define _COMPUTE_RDMA_H_

#include <rdma/rdma_cma.h>

#include "common/rdma.h"
#include "common/handshake.h"

struct rdma_client {
  struct rdma_event_channel* rdma_events;
  struct rdma_connection* conn;
  struct memory_info mem;
};

struct rdma_client* rdma_client_connect(struct sockaddr* addr);
int rdma_client_free(struct rdma_client* client);

#endif
