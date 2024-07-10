// Initial TCP control plane before RDMA

#ifndef _COMMON_RDMA_H_
#define _COMMON_RDMA_H_

#include <assert.h>
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
static inline int expect_event(struct rdma_event_channel* events, enum rdma_cm_event_type type,
                               struct rdma_cm_id** conn_id) {
  struct rdma_cm_event* ev;
  try(rdma_get_cm_event(events, &ev), "cannot get CM event");
  if (ev->event != type) {
    fprintf(stderr, "expected %s, got %s\n", rdma_event_str(type), rdma_event_str(ev->event));
    rdma_ack_cm_event(ev);
    return -1;
  }
  fprintf(stderr, "ayy!! %s\n", rdma_event_str(type));
  if (conn_id) *conn_id = ev->id;
  rdma_ack_cm_event(ev);
  return 0;
}

// Current version seems to only need RDMA read and write, without control messages. Reserved for
// future use.
struct message {
  uint8_t _reserved[16];
};

struct rdma_connection {
  struct rdma_cm_id* id;                // RDMA communication manager ID, like socket
  struct ibv_pd* pd;                    // protection domain
  struct ibv_comp_channel* cc;          // completion channel, for notification
  struct ibv_cq* cq;                    // completion queue
  struct message *send_buf, *recv_buf;  // message buffers
  struct ibv_mr *send_mr, *recv_mr;     // message buffers' registered memory regions
};

static inline int rdma_conn_free(struct rdma_connection* conn) {
  if (conn->id) {
    rdma_destroy_qp(conn->id);
    try(rdma_destroy_id(conn->id), "failed to destroy connection ID");
  }

  if (conn->send_mr)
    try_e(ibv_dereg_mr(conn->send_mr), "failed to deregister message send buffer MR");
  if (conn->recv_mr)
    try_e(ibv_dereg_mr(conn->recv_mr), "failed to deregister message recv buffer MR");
  if (conn->send_buf) free(conn->send_buf);
  if (conn->recv_buf) free(conn->recv_buf);

  if (conn->cq) try_e(ibv_destroy_cq(conn->cq), "failed to destroy completion queue");
  if (conn->cc) try_e(ibv_destroy_comp_channel(conn->cc), "failed to destroy completion channel");
  if (conn->pd) try_e(ibv_dealloc_pd(conn->pd), "failed to deallocate protection domain");

  free(conn);
  return 0;
}

// use_event: true to enable completion event notification that could be used in epoll, false to
// use busy poll and get lower latency
static inline struct rdma_connection* rdma_conn_create(struct rdma_cm_id* id, bool use_event) {
  struct rdma_connection* c = try2_p(calloc(1, sizeof(*c)));
  c->id = id;
  c->pd = try3_p(ibv_alloc_pd(id->verbs), "cannot allocate protection domain");

  if (use_event) {
    c->cc = try3_p(ibv_create_comp_channel(id->verbs), "cannot create completion channel");
    c->cq = try3_p(ibv_create_cq(id->verbs, 16, NULL, c->cc, 0), "cannot create completion queue");
    try3(ibv_req_notify_cq(c->cq, false), "cannot request for completion queue notification");
  } else {
    c->cq = try3_p(ibv_create_cq(id->verbs, 16, NULL, NULL, 0), "cannot create completion queue");
  }

  c->send_buf = try3_p(calloc(1, sizeof(*c->send_buf)));
  c->recv_buf = try3_p(calloc(1, sizeof(*c->recv_buf)));
  c->send_mr = try3_p(ibv_reg_mr(c->pd, c->send_buf, sizeof(*c->recv_buf), IBV_ACCESS_LOCAL_WRITE));
  c->recv_mr = try3_p(ibv_reg_mr(c->pd, c->recv_buf, sizeof(*c->recv_buf), IBV_ACCESS_LOCAL_WRITE));

  // Create queue pair inside CM ID
  struct ibv_qp_init_attr attr = {
    .qp_type = IBV_QPT_RC,
    .send_cq = c->cq,
    .recv_cq = c->cq,
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

static inline int _rdma_conn_poll_evented(struct rdma_connection* conn, struct ibv_wc* wc_list,
                                          size_t wc_len, unsigned int ack_count) {
  struct ibv_cq* cq_ptr;
  void* cq_ctx_ptr;
  try(ibv_get_cq_event(conn->cc, &cq_ptr, &cq_ctx_ptr), "failed to get completion event");
  assert(cq_ptr == conn->cq);

  int polled_len = 0;
  do {
    int count = try(ibv_poll_cq(conn->cq, wc_len - polled_len, wc_list + polled_len),
                    "failed to poll completion queue");
    if (count == 0) {
      if (polled_len == 0) {
        fprintf(stderr, "completion channel reports false positive\n");
        return -(errno = EBADMSG);
      }
      // nothing inside queue right now, register and wait for next event
      // TODO: busy poll for a certain number of times
      try(ibv_req_notify_cq(conn->cq, false), "cannot request for completion queue notification");
      return _rdma_conn_poll_evented(conn, wc_list + polled_len, wc_len - polled_len,
                                     ack_count + 1);
    }
    polled_len += count;
  } while (polled_len < wc_len);

  ibv_ack_cq_events(conn->cq, ack_count);  // ACKs batched here
  try(ibv_req_notify_cq(conn->cq, false), "cannot request for completion queue notification");
  return 0;
}

static inline int rdma_conn_poll(struct rdma_connection* conn, struct ibv_wc* wc_list,
                                 size_t wc_len) {
  if (conn->cc) {
    return _rdma_conn_poll_evented(conn, wc_list, wc_len, 1);
  } else {
    int polled_len = 0;
    do {
      int count = try(ibv_poll_cq(conn->cq, wc_len - polled_len, wc_list + polled_len),
                      "failed to poll completion queue");
      polled_len += count;
    } while (polled_len < wc_len);
  }
  return 0;
}

// Send data currently in send buffer.
///
// Now that we have only one buffer, we have to enable `poll_now`; the argument currently has to be
// true.
static inline int rdma_conn_send(struct rdma_connection* conn, bool poll_now) {
  try(rdma_post_send(conn->id, NULL, conn->send_buf, sizeof(*conn->send_buf), conn->send_mr,
                     IBV_SEND_SIGNALED),
      "failed to RDMA send");
  if (poll_now) {
    struct ibv_wc wc;
    try(rdma_conn_poll(conn, &wc, 1), "failed to poll");
    if (wc.status != IBV_WC_SUCCESS) {
      fprintf(stderr, "RDMA send failed: %s\n", ibv_wc_status_str(wc.status));
      return -(errno = EBADMSG);
    }
  }
  return 0;
}

// For `poll_now`, see above.
static inline int rdma_conn_recv(struct rdma_connection* conn, bool poll_now) {
  if (poll_now) {
    struct ibv_wc wc;
    try(rdma_conn_poll(conn, &wc, 1), "failed to poll");
    if (wc.status != IBV_WC_SUCCESS) {
      fprintf(stderr, "RDMA recv failed: %s\n", ibv_wc_status_str(wc.status));
      return -(errno = EBADMSG);
    }
  }
  try(rdma_post_recv(conn->id, NULL, conn->recv_buf, sizeof(*conn->recv_buf), conn->recv_mr),
      "failed to RDMA recv");
  return 0;
}

#endif
