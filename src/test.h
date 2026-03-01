/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#define t_exp(expf, exp, gotf, got, clean)                                     \
  if (exp != got) {                                                            \
    printf("FAILED %s: %s", __func__, #got);                                   \
    printf(" expected " expf " got " gotf "\n", exp, got);                     \
    clean;                                                                     \
    return;                                                                    \
  }

#define t_nexp(expf, exp, gotf, got, clean)                                    \
  if (exp == got) {                                                            \
    printf("FAILED %s: %s", __func__, #got);                                   \
    printf(" not expected " expf " got " gotf "\n", exp, got);                 \
    clean;                                                                     \
    return;                                                                    \
  }

#define t_sexp(exp, expsz, got, gotsz, clean)                                  \
  t_nexp("%p", NULL, "%p", got, clean);                                        \
  t_exp("%li", expsz, "%li", gotsz, clean);                                    \
  t_exp("%hhi", '\0', "%hhi", exp[expsz - 1], clean);                          \
  t_exp("%hhi", '\0', "%hhi", got[gotsz - 1], clean);                          \
  if (strcmp(exp, got) != 0) {                                                 \
    printf("FAILED %s: %s", __func__, #got);                                   \
    printf(" expected %s got %s\n", exp, got);                                 \
    clean;                                                                     \
  }

#define t_ok() printf("...... %s: ok\n", __func__)
