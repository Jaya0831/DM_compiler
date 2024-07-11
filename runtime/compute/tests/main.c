#include <stdio.h>

#include "../../common/try.h"
#include "context.h"

int main() {
  struct compute_context* ctx = try_p(compute_context_create(), "failed to create compute context");
  fprintf(stderr, "compute context initialized\n");

  while (1)
    ;

  try(compute_context_free(ctx), "failed to free");

  return 0;
}
