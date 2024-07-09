// RDMA client

#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../common/try.h"
#include "context.h"
#include "rdma.h"

#define RESOLVE_TIMEOUT_MS 500

int rdma_client_free(struct rdma_client* client) {
  int ret = 0;
  if (client->conn) {
    ret = rdma_conn_free(client->conn);
    if (ret < 0) perror("failed to free RDMA connection");
  }
  if (client->rdma_events) rdma_destroy_event_channel(client->rdma_events);
  free(client);
  return ret;
}

struct rdma_client* rdma_client_connect(struct sockaddr* addr) {
  struct rdma_client* client = try2_p(calloc(1, sizeof(*client)));
  struct rdma_cm_id *id = NULL, *id2;

  client->rdma_events = try3_p(rdma_create_event_channel(), "failed to create RDMA event channel");
  try3(rdma_create_id(client->rdma_events, &id, NULL, RDMA_PS_TCP), "failed to create RDMA ID");

  // Resolve remote address and route
  try3(rdma_resolve_addr(id, NULL, addr, RESOLVE_TIMEOUT_MS), "failed to resolve address");
  try3(expect_event(client->rdma_events, RDMA_CM_EVENT_ADDR_RESOLVED, NULL));
  try3(rdma_resolve_route(id, RESOLVE_TIMEOUT_MS), "failed to resolve route");
  try3(expect_event(client->rdma_events, RDMA_CM_EVENT_ROUTE_RESOLVED, NULL));

  // prevent double-free using macros
  id2 = id;
  id = NULL;
  client->conn = try3_p(rdma_conn_create(id2, false), "failed to create RDMA connection");

  // TODO: connect
  try3(rdma_connect(client->conn->id, NULL), "failed to connect");
  try3(expect_event(client->rdma_events, RDMA_CM_EVENT_ESTABLISHED, NULL));

  return client;

cleanup:
  if (id) rdma_destroy_id(id);
  rdma_client_free(client);
  return NULL;
}
