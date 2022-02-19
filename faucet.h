#ifndef FAUCET_H_
#define FAUCET_H_

// void *realloc(void *ptr, size_t new_size);
// void* calloc(size_t num, size_t size);

#include <stddef.h>

enum faucet_memory_op {
  FAUCET_MALLOC,
  FAUCET_REALLOC,
  FAUCET_CALLOC,
  FAUCET_FREE,
};

struct faucet_trace {
  void *ptr;

  char const *file;
  unsigned int line;
  enum faucet_memory_op op;
};

enum faucet_options {
  FAUCET_OPTION_DEFAULT = 0,
};

void faucet_start(enum faucet_options);
void faucet_stop();
void faucet_status();

// void* malloc(size_t size);
void *faucet_p_alloc(struct faucet_trace, void *, size_t);
// void free(void*)
void faucet_p_free(struct faucet_trace, void *);

#define malloc(size) faucet_p_alloc(faucet_trace_here(FAUCET_MALLOC), 0, size)
#define free(ptr) faucet_p_free(faucet_trace_here(FAUCET_FREE), ptr)

#define faucet_trace_here(oper)                                                \
  (struct faucet_trace) { .file = __FILE__, .line = __LINE__, .op = oper, }

#endif // FAUCET_H_
