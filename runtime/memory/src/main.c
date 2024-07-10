#include <argp.h>
#include <netinet/in.h>
#include <stdint.h>
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

int main(int argc, char** argv) {
  struct arguments args = {};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  uint16_t port = args.port_str ? try(parse_port(args.port_str), "failed to parse port") : 12345;

  // Allocate only 8 GB of memory pool while we use local environment for development
  size_t mem_pool_size = (size_t)8 << 30;

  struct memory_context* ctx =
    try_p(memory_context_create(port, mem_pool_size), "failed to initialize memory context");
  fprintf(stderr, "memory context initialized\n");

  // TODO: main loop, poll cq, respond client
  // it is probably mainly idle

  try(memory_context_free(ctx), "failed to free memory context");
  return 0;
}
