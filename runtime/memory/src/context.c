#include <infiniband/verbs.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../../common/handshake.h"
#include "../../common/try.h"
#include "main.h"

int memory_context_free(struct memory_context* ctx) {
  if (ctx->rdma) rdma_server_free(ctx->rdma);
  if (ctx->mem_pool) {
    void* mem_pool_addr = ctx->mem_pool->addr;
    size_t mem_pool_size = ctx->mem_pool->length;
    ibv_dereg_mr(ctx->mem_pool);
    munmap(mem_pool_addr, mem_pool_size);
  }
  free(ctx);
  return 0;
}

struct memory_context* memory_context_create(uint16_t listen_port, size_t mem_pool_size) {
  struct memory_context* ctx = try2_p(calloc(1, sizeof(*ctx)));
  struct ibv_pd *pd = NULL, *pd2;
  void* mem_pool_addr = NULL;

  // TODO: specify port in command line
  struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_port = htons(12345)};

  mem_pool_addr =
    try3_p(mmap(NULL, mem_pool_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0),
           "failed to mmap");
  ctx->mem_pool =
    try3_p(ibv_reg_mr(pd, mem_pool_addr, mem_pool_size,
                      IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE));
  mem_pool_addr = NULL;

  int page_size = 4096;  // TODO: get it from system
  struct memory_handshake hs = {
    .addr = (uintptr_t)ctx->mem_pool->addr,
    .rkey = ctx->mem_pool->rkey,
    .page_size = page_size,
    .page_count = mem_pool_size / page_size,
  };

  pd2 = pd;
  pd = NULL;
  ctx->rdma = try3_p(rdma_server_create(pd2, (struct sockaddr*)&addr, &hs, sizeof(hs)),
                     "failed to create RDMA server");
  return ctx;

cleanup:
  if (mem_pool_addr) munmap(mem_pool_addr, mem_pool_size);
  if (pd) ibv_dealloc_pd(pd);
  memory_context_free(ctx);
  return NULL;
}
