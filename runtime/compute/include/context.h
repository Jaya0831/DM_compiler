#ifndef _COMPUTE_CONTEXT_H_
#define _COMPUTE_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Compute-side context used during DM application runtime.
struct compute_context;

// Create compute context, and connect to memory server.
struct compute_context* compute_context_create();
int compute_context_free(struct compute_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
