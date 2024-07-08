#ifndef _MEMORY_CONTEXT_H_
#define _MEMORY_CONTEXT_H_

#include <rdma/rdma_cma.h>

#include "../../common/rdma.h"

struct memory_context {
  struct rdma_server {
    struct rdma_event_channel* rdma_events;
    struct rdma_cm_id* listen_id;
    struct rdma_connection* conn;
  } rdma;
  // TODO: chunks
};

#endif
