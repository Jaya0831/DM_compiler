#ifndef _COMPUTE_CONTEXT_H_
#define _COMPUTE_CONTEXT_H_

// Compute-side context used during DM application runtime.
struct compute_context;

// Create compute context, and connect to memory server.
struct compute_context* compute_context_connect();
int compute_context_free(struct compute_context* ctx);

#endif
