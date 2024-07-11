#include <argp.h>
#include <infiniband/verbs.h>
#include <netinet/in.h>
#include <rdma/rdma_cma.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/mman.h>

#include "../../common/parse.h"
#include "main.h"

static const char args_doc[] = "ADDRESS";
static const char doc[] = "Memory server for the disaggregated memory compiler project";

static struct argp_option options[] = {
  {"verbose", 'v', NULL, 0, "Produce verbose output"},
  {"port", 'p', "PORT", 0, "Specify port to listen; defaults to 12345"},
  {},
};

static int parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* args = state->input;
  switch (key) {
    case 'v':
      args->verbose = 1;
      break;
    case 'p':
      args->port_str = arg;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

// TODO: properly do cleanup
int main(int argc, char** argv) {
  struct arguments args = {};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  uint16_t port = args.port_str ? try(parse_port(args.port_str), "failed to parse port") : 12345;

  // Allocate only 1 GB of memory pool while we use local environment for development
  //
  // Need to set rlimit memlock high so entire memory region could be locked; could be done by
  // setting CAP_SYS_RESOURCE to the executable and call `setrlimit`
  size_t mem_pool_size = (size_t)1 << 30;

  struct memory_context* ctx =
    try_p(memory_context_create(port, mem_pool_size), "failed to initialize memory context");
  fprintf(stderr, "memory context initialized\n");

  int epfd = try(epoll_create1(0), "failed to create epoll fd");
  struct epoll_event ev, events[2];

  int rdma_events_fd = ctx->rdma->events->fd;
  ev = (typeof(ev)){.events = EPOLLIN, .data.fd = rdma_events_fd};
  try(epoll_ctl(epfd, EPOLL_CTL_ADD, rdma_events_fd, &ev), "failed to add fd to epoll");

  int ccfd = ctx->rdma->conn->cc->fd;
  ev = (typeof(ev)){.events = EPOLLIN, .data.fd = ccfd};
  try(epoll_ctl(epfd, EPOLL_CTL_ADD, ccfd, &ev), "failed to add fd to epoll");

  while (true) {
    int nfds = try(epoll_wait(epfd, events, 2, -1), "failed to epoll");
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == rdma_events_fd) {
        struct rdma_cm_event* rdma_event;
        try(rdma_get_cm_event(ctx->rdma->events, &rdma_event), "failed to get RDMA event");
        switch (rdma_event->event) {
          case RDMA_CM_EVENT_DISCONNECTED:
            fprintf(stderr, "compute side disconnected, exiting\n");
            return 0;
          default:
            fprintf(stderr, "received new RDMA event %s\n", rdma_event_str(rdma_event->event));
            break;
        }
        rdma_ack_cm_event(rdma_event);

      } else if (events[i].data.fd == ccfd) {
        struct rdma_connection* c = ctx->rdma->conn;
        struct ibv_wc wcs[MAX_POLL];
        int polled = try(rdma_conn_poll_ev(c, wcs, MAX_POLL), "failed to poll");

        bool errored = false;
        for (int i = 0; i < polled; i++) {
          if (wcs[i].status != IBV_WC_SUCCESS) {
            fprintf(stderr, "recv completion queue received error: %s\n",
                    ibv_wc_status_str(wcs[i].status));
            errored = true;
          }
          if (!errored) {
            // refill recv queue
            // currently we only have 1 slot of recv buffer; will use wr_id to indicate which slot
            // to refill.
            try(rdma_post_recv(c->id, NULL, c->recv_buf, sizeof(*c->recv_buf), c->recv_mr),
                "failed to RDMA recv");
          }
        }
        if (errored) return -1;

      } else {
        fprintf(stderr, "unknown fd %d in epoll\n", events[i].data.fd);
        return -1;
      }
    }
  }

  try(memory_context_free(ctx), "failed to free memory context");
  return 0;
}
