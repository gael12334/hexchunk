/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "error.h"
#include "typedef.h"

/*******************************************************************************
 *                                  Constants
 *******************************************************************************/

/*******************************************************************************
 *                               Type definitions
 *******************************************************************************/

typedef struct {
  void* address;
  size_t count;
} arrayview_t;

#define arrayviewof_t(T)                                                                 \
  struct {                                                                               \
    T* address;                                                                          \
    size_t count;                                                                        \
  }

typedef arrayview_t vaargs_t;
#define vaargsof_t(T) arrayviewof_t(T)

typedef arrayviewof_t(char) charview_t;
typedef arrayviewof_t(wchar_t) wcharview_t;

/*******************************************************************************
 *                                   Macros
 *******************************************************************************/
#define avof_foreach(var, av, code)                                                      \
  {                                                                                      \
    for (size_t av_i = 0; av_i < (av)->count; av_i++) {                                  \
      var = (av)->address + av_i;                                                        \
      code                                                                               \
    }                                                                                    \
  }

#define avof_cast(a)                                                                     \
  { .address = (a)->address, .count = (a)->count }

#define valist(T, ...)                                                                   \
  (vaargs_t) {                                                                           \
    .address = (T[]){__VA_ARGS__}, .count = sizeof((T[]){__VA_ARGS__}) / sizeof(T)       \
  }

#define cvtext(x)                                                                        \
  (charview_t) {                                                                         \
    .address = x, .count = sizeof(x)                                                     \
  }
#define cvsub(start, nb_of_char)                                                         \
  (charview_t) {                                                                         \
    .address = start, .count = (nb_of_char) + 1                                          \
  }

/*******************************************************************************
 *                                  Functions
 *******************************************************************************/

int av_alloc(arrayview_t* av, size_t n, size_t s);
#define avof_alloc(av, n) av_alloc((arrayview_t*)(av), n, sizeof(*(av)->address))

int av_free(arrayview_t* av, size_t s);
#define avof_free(av) av_free((arrayview_t*)(av), sizeof(*(av)->address))
