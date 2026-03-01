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
typedef enum { ae_ok, ae_err, ae_closed, ae_open, ae_null, ae_unknown } ae_t;

/*
 * App command object
 */
typedef struct {
  cstr name;
  cstr description;
  ad_t delegate;
} ac_t;

#define a_command(pname, pdesc, pdelegate)                                     \
  (ac_t){.name = pname, .description = pdesc, .delegate = pdelegate};

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
 * App object
 */
typedef struct app {
  char name[64];
  size_t namelen;
  av_t version;
  ac_t *list;
  size_t num;
  char input[1024];
  ae_t closed;
} app_t;

/*******************************************************************************
 *                            App functions
 *******************************************************************************/

/*
 * Init an app instance
 */
ae_t a_init(app_t *out, cstr name, av_t *version, ac_t *list, size_t num);

/*
 * Deinit an app instance
 */
ae_t a_deinit(app_t *a);

/*
 * Dispatch command
 */
ae_t a_dispatch(app_t *a, cstr name, ac_t *list, size_t num, aa_t *args);

/*
 * Run the app with shell arguments
 */
ae_t a_run(app_t *a);

/*
 * Get app closed status
 */
ae_t a_closed(app_t *a);
