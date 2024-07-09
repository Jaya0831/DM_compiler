#include <netinet/in.h>
#include <stdlib.h>

#include "../../common/try.h"
#include "main.h"

int memory_context_free(struct memory_context* ctx) {
  // TODO
  if (ctx->rdma) rdma_server_free(ctx->rdma);
  free(ctx);
  return 0;
}

struct memory_context* memory_context_create() {
  struct memory_context* ctx = try2_p(calloc(1, sizeof(*ctx)));

  struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(12345)};
  ctx->rdma = try3_p(rdma_server_create((struct sockaddr*)&addr), "failed to create RDMA server");

  // TODO

  return ctx;

cleanup:
  memory_context_free(ctx);
  return NULL;
}
