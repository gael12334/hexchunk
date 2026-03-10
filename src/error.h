/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 *                                  Constants
 *******************************************************************************/
#define e_calltrace_size (64)

enum {
  __OK__,
  ENULPTR,
  ESIGSEG,
  EBADOBJ,
  EMEMALL,
  ENOTIMP,
  ETHROWN,
  EUNKERR,
};

/*******************************************************************************
 *                               Type definitions
 *******************************************************************************/
typedef struct {
  int32_t code;
  uint32_t line;
  const char* name;
} here_t;

typedef struct {
  char msg[240];
} errmsg_t;

typedef struct {
  errmsg_t errmsg;
  here_t here;
} error_t;

typedef struct {
  const error_t list[e_calltrace_size];
  const uint64_t count;
} calltrace_t;

/*******************************************************************************
 *                                  Macros
 *******************************************************************************/
#define e_here(c) ((here_t){.code = c, .line = __LINE__, .name = __FILE_NAME__})
#define e_report(c, f, ...) e_pushtrace(e_here(c), f __VA_OPT__(, __VA_ARGS__))
#define e_throw(c, f, ...) return e_report(c, "%s: " f, #c __VA_OPT__(, __VA_ARGS__))
#define e_try(fn, cleanup)                                                               \
  if (0 != fn) {                                                                         \
    cleanup;                                                                             \
    e_report(e_lastcode(), "Fault caught !");                                            \
    goto e_lcl_catch_label;                                                              \
  }
#define e_trycatch(fn, cleanup)                                                          \
  if (0 != fn) {                                                                         \
    cleanup;                                                                             \
  }
#define e_catch(var, code)                                                               \
  goto e_endlcl_catch_label;                                                             \
  e_lcl_catch_label: {                                                                   \
    var = e_calltrace();                                                                 \
    code;                                                                                \
  }                                                                                      \
  e_endlcl_catch_label:

#define e_throw_NULPTR(var) e_throw(ENULPTR, "value of '%s' was '%p'.", #var, var)
#define e_throw_SIGSEG(var) e_throw(ESIGSEG, "ptr '%s' was '%p'.", #var, var)
#define e_throw_BADOBJ(var) e_throw(EBADOBJ, "state of '%s' is invalid.", #var)
#define e_throw_MEMALL() e_throw(EMEMALL, "memory allocation returned NULL.")
#define e_throw_NOTIMP() e_throw(ENOTIMP, "%s is not implemented yet.", __func__)
#define e_throw_THROWN() e_throw(ETHROWN, "callee has thrown.")
#define e_throw_UNKERR() e_throw(EUNKERR, "unknown error.");

/*******************************************************************************
 *                                  Functions
 *******************************************************************************/
void e_clear(void);
int e_lastcode(void);
calltrace_t* e_calltrace(void);
int e_pushtrace(here_t here, const char* format, ...);
void e_printtrace(FILE* out);
