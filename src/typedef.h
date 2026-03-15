/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef const char* cstr_t;

/*
 * Value types
 */
enum vtype {
  v_int,
  v_num,
  v_str,
  v_buf,
};

/*
 * Value union
 */
union vunion {
  int64_t i64;
  double f64;
  char* str;
  void* buf;
};

/*
 * Value
 */
typedef struct value {
  enum vtype type;
  union vunion poly;
  size_t size;
} value_t;
