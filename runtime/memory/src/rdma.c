#include <rdma/rdma_cma.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../common/rdma.h"
#include "main.h"

int rdma_server_free(struct rdma_server* server) {
  int ret;
  if (server->conn) {
    ret = rdma_conn_free(server->conn);
    if (ret < 0) perror("failed to free RDMA connection");
  }
  if (server->listen_id) rdma_destroy_id(server->listen_id);
  if (server->rdma_events) rdma_destroy_event_channel(server->rdma_events);
  return 0;
}

struct rdma_server* rdma_server_create(struct sockaddr* addr) {
  struct rdma_server* server = try2_p(calloc(1, sizeof(*server)));
  server->rdma_events = try3_p(rdma_create_event_channel(), "failed to create RDMA event channel");
  try3(rdma_create_id(server->rdma_events, &server->listen_id, NULL, RDMA_PS_TCP),
       "failed to create listen ID");

  // Bind and listen for client to connect
  try3(rdma_bind_addr(server->listen_id, addr), "failed to bind address");
  // fprintf(stderr, "server is listening at %s:%d\n", inet_ntoa(addr->sin_addr),
  //         ntohs(addr.sin_port));
  fprintf(stderr, "Waiting for connection from compute...\n");
  try3(rdma_listen(server->listen_id, 8), "failed to listen");

  // Get CM ID for the connection
  struct rdma_cm_id* conn_id = NULL;
  try3(expect_event(server->rdma_events, RDMA_CM_EVENT_CONNECT_REQUEST, &conn_id));
  server->conn = try3_p(rdma_conn_create(conn_id, true), "failed to create connection");

  try3(rdma_accept(server->conn->id, NULL), "failed to accept");
  try3(expect_event(server->rdma_events, RDMA_CM_EVENT_ESTABLISHED, NULL));

cleanup:
  rdma_server_free(server);
  return NULL;
}
