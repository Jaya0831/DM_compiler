#include <infiniband/verbs.h>
#include <netinet/in.h>
#include <pthread.h>
#include <rdma/rdma_verbs.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache-internal.h"
#include "common/parse.h"
#include "context-internal.h"
#include "context.h"
#include "hashmap.h"
#include "rdma.h"

// Public APIs

int compute_context_free(struct compute_context* ctx) {
  if (ctx->rdma) rdma_client_free(ctx->rdma);

  free(ctx->types);
  free(ctx->type_chunk_refs);

  for (int i = 0; i < ctx->types_count; i++) cache_block_free(&ctx->caches[i]);
  free(ctx->caches);
  hashmap_free(ctx->addr_trans_table);

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

  // 0 to 2 are reserved for arbitrary types
  ctx->types_count = 3;
  ctx->types = try3_p(calloc(ctx->types_count, sizeof(void*)));
  ctx->type_chunk_refs = try3_p(calloc(ctx->types_count, sizeof(*ctx->type_chunk_refs)));
  for (int i = 0; i < ctx->types_count; i++)
    pthread_rwlock_init(&ctx->type_chunk_refs[i].lock, NULL);
  ctx->next_chunk = ctx->rdma->mem.addr;

  // TODO: cache blocks for oblivious types
  ctx->caches = try3_p(calloc(ctx->types_count, sizeof(struct cache_block)));
  ctx->addr_trans_table = addr_trans_table_new();

  // printf("client mem addr: %lx; rkey: %u\n", ctx->rdma->mem.addr, ctx->rdma->mem.rkey);

  // Dirty code to test whether and how RDMA read/write works
  // uint64_t buf = 114514;
  // struct ibv_mr* mr =
  //   try3_p(ibv_reg_mr(ctx->rdma->conn->pd, &buf, sizeof(buf),
  //                     IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE),
  //          "ibv_reg_mr");

  // try3(rdma_post_write(ctx->rdma->conn->id, NULL, &buf, sizeof(buf), mr, IBV_SEND_SIGNALED,
  //                      ctx->rdma->mem.addr, ctx->rdma->mem.rkey),
  //      "rdma_post_read");
  // struct ibv_wc wc;
  // while (try3(ibv_poll_cq(ctx->rdma->conn->send_cq, 1, &wc), "rdma_get_send_comp") == 0)
  //   ;
  // printf("done writing! wc.status = %s, buf = %lu\n", ibv_wc_status_str(wc.status), buf);

  // buf = 0;
  // printf("buf = %lu", buf);
  // try3(rdma_post_read(ctx->rdma->conn->id, NULL, &buf, sizeof(buf), mr, IBV_SEND_SIGNALED,
  //                     ctx->rdma->mem.addr, ctx->rdma->mem.rkey),
  //      "rdma_post_read");
  // // struct ibv_wc wc;
  // while (try3(ibv_poll_cq(ctx->rdma->conn->send_cq, 1, &wc), "rdma_get_send_comp") == 0)
  //   ;
  // printf("done reading! wc.status = %s, buf = %lu\n", ibv_wc_status_str(wc.status), buf);

  // TODO: init chunk, cache, ...

  return ctx;

cleanup:
  compute_context_free(ctx);
  return NULL;
}
