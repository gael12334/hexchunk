/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#include "stream.h"

typedef enum {
  ce_ok,
} ce_t;

typedef union {
  int64_t si;
  uint64_t ui;
  double lf;
  str cp;
} c_value_t;

typedef struct {
  c_value_t *args;
  size_t count;
} c_args_t;

typedef enum : int64_t {
  c_format_signed,
  c_format_unsigned,
  c_format_float,
  c_format_string,
  c_format_num = 6,
} c_format_t;

typedef struct {
  cstr name;
  c_format_t format[c_format_num];
  ce_t (*func)(c_args_t *args, void *extra);
} command_t;

#define c_wrap(ffunc)                                                          \
  (command_t) { .name = #ffunc, .func = ffunc }
