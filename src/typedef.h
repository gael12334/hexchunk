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
struct value {
  enum vtype type;
  union vunion cvu;
  size_t size;
};

/*
 * Type ignorant arrayview
 */
typedef struct {
  void* address;
  size_t count;
} arrayview_t;

/*
 * Generic macro of arrayview
 */
#define arrayviewof_t(T)                                                                 \
  struct {                                                                               \
    T* address;                                                                          \
    size_t count;                                                                        \
  }

/*
 * For each element in list
 */
#define avof_foreach(var, av)                                                            \
  for (var = (av)->address; var != (av)->address + (av)->count; var++)
