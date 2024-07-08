#ifndef _COMPUTE_RDMA_H_
#define _COMPUTE_RDMA_H_

#include <rdma/rdma_cma.h>

#include "context.h"

struct rdma_client* rdma_client_connect(struct sockaddr* addr);
int rdma_client_free(struct rdma_client* client);

#endif
