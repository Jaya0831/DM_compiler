// Initial TCP control plane before RDMA

#ifndef _COMMON_RDMA_H_
#define _COMMON_RDMA_H_

#include <errno.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "try.h"

// Wait for a certain type of CM event, regarding others as error.
static inline int _expect_event(struct rdma_event_channel* events, enum rdma_cm_event_type type,
                                struct rdma_cm_id** conn_id, struct rdma_conn_param* param) {
  struct rdma_cm_event* ev;
  try(rdma_get_cm_event(events, &ev), "cannot get CM event");
  if (ev->event != type) {
    fprintf(stderr, "expected %s, got %s\n", rdma_event_str(type), rdma_event_str(ev->event));
    rdma_ack_cm_event(ev);
    return -1;
  }
  fprintf(stderr, "ayy!! %s\n", rdma_event_str(type));
  if (conn_id) *conn_id = ev->id;
  if (param) *param = ev->param.conn;
  rdma_ack_cm_event(ev);
  return 0;
}

static inline int expect_event(struct rdma_event_channel* events, enum rdma_cm_event_type type) {
  return _expect_event(events, type, NULL, NULL);
}

static inline int expect_connect_request(struct rdma_event_channel* events,
                                         struct rdma_cm_id** conn_id,
                                         struct rdma_conn_param* param) {
  return _expect_event(events, RDMA_CM_EVENT_CONNECT_REQUEST, conn_id, param);
}

static inline int expect_established(struct rdma_event_channel* events,
                                     struct rdma_conn_param* param) {
  return _expect_event(events, RDMA_CM_EVENT_ESTABLISHED, NULL, param);
}

// Current version seems to only need RDMA read and write, without control messages. Reserved for
// future use.
struct message {
  uint8_t _reserved[16];
};

//
struct rdma_connection {
  struct rdma_cm_id* id;  // RDMA communication manager ID, like socket
  struct ibv_pd* pd;      // protection domain

  struct ibv_comp_channel* cc;       // receive completion channel, for event notification
  struct ibv_cq *send_cq, *recv_cq;  // completion queues

  struct message *send_buf, *recv_buf;  // message buffers
  struct ibv_mr *send_mr, *recv_mr;     // message buffers' registered memory regions
};

static inline int rdma_conn_free(struct rdma_connection* conn) {
  if (conn->id) {
    rdma_destroy_qp(conn->id);
    rdma_destroy_id(conn->id);
  }

  if (conn->send_mr) ibv_dereg_mr(conn->send_mr);
  if (conn->recv_mr) ibv_dereg_mr(conn->recv_mr);
  if (conn->send_buf) free(conn->send_buf);
  if (conn->recv_buf) free(conn->recv_buf);

  if (conn->send_cq) ibv_destroy_cq(conn->send_cq);
  if (conn->recv_cq) ibv_destroy_cq(conn->recv_cq);
  if (conn->cc) ibv_destroy_comp_channel(conn->cc);
  if (conn->pd) ibv_dealloc_pd(conn->pd);

  free(conn);
  return 0;
}

// use_event: true to enable completion event notification that could be used in epoll, false to
// use busy poll and get lower latency
static inline struct rdma_connection* rdma_conn_create(struct rdma_cm_id* id, bool use_event) {
  struct rdma_connection* c = try2_p(calloc(1, sizeof(*c)));
  c->id = id;
  c->pd = try3_p(ibv_alloc_pd(c->id->verbs), "cannot allocate protection domain");

  c->send_cq =
    try3_p(ibv_create_cq(id->verbs, 16, NULL, NULL, 0), "cannot create completion queue");
  if (use_event) {
    c->cc = try3_p(ibv_create_comp_channel(id->verbs), "cannot create completion channel");
    c->recv_cq =
      try3_p(ibv_create_cq(id->verbs, 16, NULL, c->cc, 0), "cannot create completion queue");
    try3(ibv_req_notify_cq(c->recv_cq, false), "cannot arm completion channel");
  } else {
    c->recv_cq =
      try3_p(ibv_create_cq(id->verbs, 16, NULL, NULL, 0), "cannot create completion queue");
  }

  c->send_buf = try3_p(calloc(1, sizeof(*c->send_buf)));
  c->recv_buf = try3_p(calloc(1, sizeof(*c->recv_buf)));
  c->send_mr = try3_p(ibv_reg_mr(c->pd, c->send_buf, sizeof(*c->recv_buf), IBV_ACCESS_LOCAL_WRITE));
  c->recv_mr = try3_p(ibv_reg_mr(c->pd, c->recv_buf, sizeof(*c->recv_buf), IBV_ACCESS_LOCAL_WRITE));

  // Create queue pair inside CM ID
  struct ibv_qp_init_attr attr = {
    .qp_type = IBV_QPT_RC,
    .send_cq = c->send_cq,
    .recv_cq = c->recv_cq,
    .cap = {.max_send_wr = 32, .max_recv_wr = 4, .max_send_sge = 1, .max_recv_sge = 1},
  };
  try3(rdma_create_qp(id, c->pd, &attr), "cannot create queue pair");

  // pre-post recv
  try3(rdma_post_recv(c->id, NULL, c->recv_buf, sizeof(*c->recv_buf), c->recv_mr),
       "failed to RDMA recv");

  return c;

cleanup:
  rdma_conn_free(c);
  return NULL;
}

// Poll as many work completions as available in one event.
static inline int rdma_conn_poll_ev(struct rdma_connection* conn, struct ibv_wc* wcs, size_t wcn) {
  struct ibv_cq* cq_ptr;
  void* cq_ctx_ptr;
  try(ibv_get_cq_event(conn->cc, &cq_ptr, &cq_ctx_ptr), "failed to get completion event");
  // assert(cq_ptr == conn->recv_cq);

  int polled = 0;
  do {
    int count =
      try(ibv_poll_cq(cq_ptr, wcn - polled, wcs + polled), "failed to poll completion queue");
    if (count == 0) {
      if (polled == 0) {
        fprintf(stderr, "completion channel reports false positive\n");
        return -(errno = EBADMSG);
      }
      break;
    }
    polled += count;
  } while (polled < wcn);

  ibv_ack_cq_events(cq_ptr, 1);  // potential optimization by batching ACKs
  try(ibv_req_notify_cq(cq_ptr, false), "cannot rearm completion channel");
  return polled;
}

#endif
