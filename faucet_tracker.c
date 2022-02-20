#include "faucet_internal.h"

#include <stdio.h>
#include <stdlib.h>

static void error(char const *c) { fprintf(stderr, "%s\n", c); }

struct faucet_trace_tracker
faucet_trace_tracker_init(struct faucet_trace *trace_storage, size_t capacity) {
  return (struct faucet_trace_tracker){
      .capacity = capacity,
      .traces = trace_storage,
  };
}

void faucet_trace_tracker_add(struct faucet_trace_tracker *tt,
                              struct faucet_trace t) {
  if (tt->size + 1 >= tt->capacity) {
    // force a crash if we can't add the trace
    error("[FAUCET] faucet_trace_tracker_add: Can't add trace");
    abort();
  }

  tt->traces[tt->size] = t;
  tt->size += 1;
}

void faucet_trace_tracker_remove(struct faucet_trace_tracker *tt, void *ptr) {
  if (tt->size < 0) {
    // force a crash if there are no traces
    error("[FAUCET] faucet_trace_tracker_remove: No traces to remove");
    abort();
  }

  // find the trace related to the given pointer
  for (size_t i = 0; i < tt->size; ++i) {
    struct faucet_trace_internal *in = tt->traces[i].in;
    if (in->ptr == ptr) {
      // take the last element of the array
      // and place it at this index
      tt->traces[i] = tt->traces[tt->size - 1];
      tt->size -= 1;
      free(in);
      return;
    }
  }

  // no trace found, force a crash
  abort();
  error("[FAUCET] faucet_trace_tracker_remove: Tried freeing untracked trace");
}
