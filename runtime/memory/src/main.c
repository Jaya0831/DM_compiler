#include <argp.h>

#include "main.h"

static const char args_doc[] = "";
static const char doc[] = "Memory server for the disaggregated memory compiler project";

static struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output"},
  {},
};

static int parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = state->input;
  switch (key) {
    case 'v':
      args->verbose = 1;
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
}
