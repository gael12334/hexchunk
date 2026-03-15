/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "arrayview.h"

/*******************************************************************************
 *                                  Functions
 *******************************************************************************/

int av_alloc(arrayview_t* av, size_t n, size_t s) {
  if (av == NULL)
    e_throw_NULPTR(av);

  av->address = malloc(n * s);
  if (av->address == NULL)
    e_throw_MEMALL();

  av->count = n;
  return __OK__;
}

int av_free(arrayview_t* av, size_t s) {
  if (av == NULL)
    e_throw_NULPTR(av);

  if (av->address == NULL)
    e_throw_BADOBJ(av->addres);

  memset(av->address, 0, av->count * s);
  free(av->address);
  av->address = NULL;
  av->count = 0;
  return __OK__;
}
