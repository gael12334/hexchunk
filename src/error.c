/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "error.h"

#include <pthread.h>
#include <stdarg.h>

union ct {
  struct {
    error_t list[e_calltrace_size];
    uint64_t count;
  };

  calltrace_t readonly;
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static union ct e_trace = {0};

void e_clear(void) {
  pthread_mutex_lock(&lock);
  e_trace.count = 0;
  pthread_mutex_unlock(&lock);
}

calltrace_t* e_calltrace(void) {
  return &e_trace.readonly;
}

int e_lastcode(void) {
  int code = 0;
  pthread_mutex_lock(&lock);

  if (e_trace.count > 0) {
    error_t* err = &e_trace.list[e_trace.count - 1];
    code = err->here.code;
  }

  pthread_mutex_unlock(&lock);
  return code;
}

int e_pushtrace(here_t here, const char* format, ...) {
  va_list v;
  va_start(v, format);
  pthread_mutex_lock(&lock);

  // if call depth exceeds queue size, overwrite last error.
  if (e_trace.count >= e_calltrace_size)
    e_trace.count = e_calltrace_size - 1;

  if (format == NULL)
    format = "DEFAULT_STRING";

  error_t* err = &e_trace.list[e_trace.count];
  e_trace.count++;
  vsnprintf(err->errmsg.msg, sizeof(err->errmsg) - 1, format, v);
  err->here = here;

  pthread_mutex_unlock(&lock);
  va_end(v);
  return here.code;
}

void e_printtrace(FILE* out) {
  if (out == NULL)
    return;

  calltrace_t* ct = e_calltrace();
  for (size_t i = 0; i < ct->count; i++) {
    const here_t* here = &ct->list[i].here;
    const errmsg_t* errmsg = &ct->list[i].errmsg;
    fprintf(out, "[%s : %i] %s\n", here->name, here->line, errmsg->msg);
  }
}
