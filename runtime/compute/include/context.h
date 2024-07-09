#ifndef _COMPUTE_CONTEXT_H_
#define _COMPUTE_CONTEXT_H_

#include <rdma/rdma_cma.h>

#include "../../common/rdma.h"

// Compute-side context used during DM application runtime.
struct compute_context {
  struct rdma_client {
    struct rdma_event_channel* rdma_events;
    struct rdma_connection* conn;
  } rdma;
  // TODO: cache, chunks ref, ...
};

#endif
