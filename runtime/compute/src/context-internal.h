#ifndef _COMPUTE_CONTEXT_INTERNAL_H_
#define _COMPUTE_CONTEXT_INTERNAL_H_

#include "rdma.h"

// Internal representation of compute context
struct compute_context {
  struct rdma_client* rdma;
  // TODO: cache, chunks ref, ...
};

#endif
