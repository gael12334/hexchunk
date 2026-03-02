/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "stream.h"

struct app;

/*******************************************************************************
 *                            App object definitions
 *******************************************************************************/

/*
 * App arguments
 */
typedef struct {
  size_t argc;
  str *argv;
} aa_t;

#define a_args(pargc, pargv)                                                   \
  (aa_t) { .argc = pargc, .argv = pargv }

/*
 * App command delegate
 */
typedef int (*ad_t)(struct app *a, aa_t *args);

/*
 * App error codes
 */
typedef enum {
  ae_ok,
  ae_err,
  ae_closed,
  ae_opened,
  ae_null,
  ae_unknown,
  ae_number,
  ae_stdlib,
  ae_size,
} ae_t;

/*
 * App command object
 */
typedef struct {
  cstr name;
  cstr description;
  ad_t delegate;
} ac_t;

#define a_command(pname, pdesc, pdelegate)                                     \
  (ac_t) { .name = pname, .description = pdesc, .delegate = pdelegate }

/*
 * App version object
 */
typedef struct {
  uint64_t major;
  uint64_t minor;
  uint64_t revision;
} av_t;

#define a_version(pmajor, pminor, prev)                                        \
  (av_t) { .major = pmajor, .minor = pminor, .revision = prev }

/*
 * App parameter object
 */

typedef struct {
  char name[64];
  av_t version;
  ac_t *commands;
  size_t cmdnum;
} ap_t;

/*
 * App object
 */
typedef struct app {
  // arguments
  str *argbuf;
  size_t argalloc;

  // commands
  ac_t *cmdbuf;
  size_t cmdnum;

  // description
  char name[64];
  av_t version;

  // input
  char input[1024];
  FILE *istream;

  // state
  ae_t closed;

  // last cmd result
  int result;
} app_t;

/*******************************************************************************
 *                            App functions
 *******************************************************************************/

/*
 * Init an app instance
 */
ae_t a_init(app_t *out, ap_t *params);

/*
 * Deinit an app instance
 */
ae_t a_deinit(app_t *a);

/*
 * Dispatch command
 */
ae_t a_dispatch(app_t *a, cstr name, ac_t *list, size_t num, aa_t *args);

/*
 * Prompt user input
 */
ae_t a_prompt(app_t *a, aa_t *out);

/*
 * Check if app is closed
 */
ae_t a_closed(app_t *a);

/*
 * Parse an integer
 */
ae_t a_arg2long(cstr arg, long *out);
