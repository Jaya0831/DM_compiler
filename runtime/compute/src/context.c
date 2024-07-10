#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../common/parse.h"
#include "context-internal.h"
#include "context.h"
#include "rdma.h"

int compute_context_free(struct compute_context* ctx) {
  if (ctx->rdma) try(rdma_client_free(ctx->rdma), "failed to free client");
  free(ctx);
  return 0;
}

struct compute_context* compute_context_create() {
  char* addr_str = getenv("MEMORY_ADDR");
  if (!addr_str) {
    fprintf(stderr, "please set MEMORY_ADDR environment variable to memory server address\n");
    errno = EINVAL;
    return NULL;
  }
  struct sockaddr_storage addr = {};
  try2(parse_addr(addr_str, &addr, 12345), "failed to parse network address");

  struct compute_context* ctx = try2_p(calloc(1, sizeof(*ctx)));
  ctx->rdma =
    try3_p(rdma_client_connect((struct sockaddr*)&addr), "failed to connect to memory server");

  // TODO: init cache

  return ctx;

cleanup:
  compute_context_free(ctx);
  return NULL;
}
