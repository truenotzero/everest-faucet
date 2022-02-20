#include "faucet_internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static struct faucet_trace_tracker tracker;
static enum faucet_options options;

void faucet_start(enum faucet_options o) {
#ifndef FAUCET_MAX_TRACES
#define FAUCET_MAX_TRACES 4096
#endif
  static struct faucet_trace traces[FAUCET_MAX_TRACES];
  tracker = faucet_trace_tracker_init(traces, FAUCET_MAX_TRACES);
  options = o;
}

void faucet_stop() {
  printf("[FAUCET] Terminating, here are them memory leaks\n");
  faucet_status();
}

struct si_byte {
  float scalar;
  char prefix;
};

struct si_byte do_si_byte(size_t b) {
  char const units[] = {
      'k',
      'M',
  };
  char u = '\0';
  float f = b;
  for (size_t i = 0; i < sizeof(units); ++i) {
    if (f / 1024 < 1) {
      break;
    }

    u = units[i];
    f /= 1024;
  }

  return (struct si_byte){
      .scalar = f,
      .prefix = u,
  };
}

void faucet_status() {
  printf("[FAUCET] Tracing active allocations...\n");
  if (options & (FAUCET_OPTION_LOG_NUM_TRACES | FAUCET_OPTION_LOG_PCT_TRACES)) {
    printf("[FAUCET] Active traces: ");
    if (options & FAUCET_OPTION_LOG_NUM_TRACES) {
      printf("%zu/%zu ", tracker.size, tracker.capacity);
    }
    if (options & FAUCET_OPTION_LOG_PCT_TRACES) {
      printf("(%02.2f%%)", (float)tracker.size / tracker.capacity);
    }
    printf("\n");
  }

  size_t total_leaked = 0;

  for (size_t i = 0; i < tracker.size; ++i) {
    struct faucet_trace e = tracker.traces[i];
    struct faucet_trace_internal *in = e.in;
    printf("[FAUCET] [%s:%d] ", e.file, e.line);

    total_leaked += in->alloc_size;

    if (options) {
      printf(" =>\t");
    }

    if (options & FAUCET_OPTION_LOG_OP) {
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
      printf("op = %s\t", op);
    }

    if (options & FAUCET_OPTION_LOG_ALLOC_SIZE) {
      struct si_byte b = do_si_byte(in->alloc_size);
      printf("qty = %.2f%cb\t", b.scalar, b.prefix);
    }

    if (options & FAUCET_OPTION_LOG_ADDRESS) {
      printf("addy = %#X\t", (unsigned int)in->ptr);
    }
    printf("\n");
  }

  if (options & FAUCET_OPTION_LOG_TOTAL_MEM_LEAKED) {
    struct si_byte b = do_si_byte(total_leaked);
    printf("[FAUCET] Total memory leaked: %.2f%c\n", b.scalar, b.prefix);
  }
}

static void add_allocated_trace(struct faucet_trace t, void *p,
                                size_t alloc_size) {
  struct faucet_trace_internal *in = malloc(sizeof(*in));
  in->ptr = p;
  in->alloc_size = alloc_size;
  t.in = in;
  faucet_trace_tracker_add(&tracker, t);
}

void *faucet_p_alloc(struct faucet_trace t, void *p, size_t size) {
  switch (t.op) {
  case FAUCET_MALLOC:
    p = malloc(size);
    break;

  case FAUCET_REALLOC:
    p = realloc(p, size);
    break;

  default:
    assert(0 && "Invalid memory operation");
    return 0;
  }

  add_allocated_trace(t, p, size);
  return p;
}

void *faucet_p_calloc(struct faucet_trace t, size_t num, size_t size) {
  void *p = 0;
  if (t.op != FAUCET_CALLOC) {
    p = calloc(num, size);
    add_allocated_trace(t, p, num * size);
  } else {
    assert(0 && "Invalid memory operation");
  }

  return p;
}

void faucet_p_free(struct faucet_trace t, void *ptr) {
  assert(t.op == FAUCET_FREE);
  faucet_trace_tracker_remove(&tracker, t, ptr);
  free(ptr);
}
