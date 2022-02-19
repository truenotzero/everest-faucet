#include "faucet_internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static struct faucet_trace_tracker tracker;

void faucet_start(enum faucet_options o) {
#ifndef FAUCET_MAX_TRACES
#define FAUCET_MAX_TRACES 4096
#endif
  static struct faucet_trace traces[FAUCET_MAX_TRACES];
  tracker = faucet_trace_tracker_init(traces, FAUCET_MAX_TRACES);
}

void faucet_stop() {
  printf("[FAUCET] Terminating, here are them memory leaks\n");
  faucet_status();
}

void faucet_status() {
  printf("[FAUCET] Memory status log:\n");
  float pct = (float)tracker.size / tracker.capacity;
  printf("[FAUCET] Active traces: %zu/%zu (%02.2f%%)\n", tracker.size,
         tracker.capacity, pct);
  for (size_t i = 0; i < tracker.size; ++i) {
    struct faucet_trace e = tracker.traces[i];
    char const *op;
    switch (e.op) {
    case FAUCET_MALLOC:
      op = "malloc";
      break;
    case FAUCET_CALLOC:
      op = "calloc";
      break;
    case FAUCET_REALLOC:
      op = "realloc";
      break;
    default:
      op = "??? (possibly a bug, check this line!)";
    }
    printf("[FAUCET] [%s:%d] => %s\n", e.file, e.line, op);
  }
}

void *faucet_p_alloc(struct faucet_trace t, void *p, size_t size) {
  switch (t.op) {
  case FAUCET_MALLOC:
    p = malloc(size);
    break;

  case FAUCET_REALLOC:
    p = realloc(p, size);
    break;

  case FAUCET_CALLOC:
    p = calloc(size, 1);
    break;

  default:
    assert(0 && "Invalid memory operation");
    return 0;
  }

  t.ptr = p;
  faucet_trace_tracker_add(&tracker, t);

  return p;
}

void *faucet_p_calloc(struct faucet_trace t, size_t num, size_t size) {
  void *p = 0;
  if (t.op != FAUCET_CALLOC) {
    p = calloc(num, size);
    t.ptr = p;
    faucet_trace_tracker_add(&tracker, t);
  } else {
    assert(0 && "Invalid memory operation");
  }

  return p;
}

void faucet_p_free(struct faucet_trace t, void *ptr) {
  assert(t.op == FAUCET_FREE);
  faucet_trace_tracker_remove(&tracker, ptr);
  free(ptr);
}
