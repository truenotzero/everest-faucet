#ifndef FAUCET_H_
#define FAUCET_H_

#include <stddef.h>

enum faucet_memory_op {
  FAUCET_MALLOC,
  FAUCET_REALLOC,
  FAUCET_CALLOC,
  FAUCET_FREE,
};

struct faucet_trace {
  char const *file;
  unsigned int line;
  enum faucet_memory_op op;

  void *in;
};

enum faucet_options {
  FAUCET_OPTION_LOG_OP = 0x01,
  FAUCET_OPTION_LOG_ALLOC_SIZE = 0x02,
  FAUCET_OPTION_LOG_ADDRESS = 0x04,
  FAUCET_OPTION_LOG_NUM_TRACES = 0x08,
  FAUCET_OPTION_LOG_PCT_TRACES = 0x10,
  FAUCET_OPTION_LOG_TOTAL_MEM_LEAKED = 0x20,

  FAUCET_OPTION_NONE = 0x00,
  FAUCET_OPTION_ALL = ~0,
};

void faucet_start(enum faucet_options);
void faucet_stop();
void faucet_status();

void *faucet_p_alloc(struct faucet_trace, void *, size_t);
void *faucet_p_calloc(struct faucet_trace, size_t, size_t);
void faucet_p_free(struct faucet_trace, void *);

// void* malloc(size_t size);
#define malloc(size) faucet_p_alloc(faucet_trace_here(FAUCET_MALLOC), 0, (size))

// void* calloc(size_t num, size_t size);
#define calloc(num, size)                                                      \
  faucet_p_alloc(faucet_trace_here(FAUCET_CALLOC), 0, (size))

// void *realloc(void *ptr, size_t new_size);
#define realloc(ptr, size)                                                     \
  faucet_p_alloc(faucet_trace_here(FAUCET_REALLOC), (ptr), (size))

// void free(void*)
#define free(ptr) faucet_p_free(faucet_trace_here(FAUCET_FREE), (ptr))

#define faucet_trace_here(oper)                                                \
  (struct faucet_trace) { .file = __FILE__, .line = __LINE__, .op = oper, }

#endif // FAUCET_H_
