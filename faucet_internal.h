#ifndef FAUCET_INTERNAL_H_
#define FAUCET_INTERNAL_H_

#include "faucet.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

struct faucet_trace_internal {
  void *ptr;
  size_t alloc_size;
};

struct faucet_trace_tracker {
  size_t capacity;
  size_t size;
  struct faucet_trace *traces;
};

struct faucet_trace_tracker faucet_trace_tracker_init(struct faucet_trace *,
                                                      size_t);
void faucet_trace_tracker_add(struct faucet_trace_tracker *,
                              struct faucet_trace);
void faucet_trace_tracker_remove(struct faucet_trace_tracker *, void *);

#endif // FAUCET_INTERNAL_H_
