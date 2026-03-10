/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "listview.h"

int lv_append(listview_t* lv, void* e, size_t s) {
  if (lv == NULL)
    e_throw_NULPTR(lv);

  if (e == NULL)
    e_throw_NULPTR(e);

  if (lv->count > lv->alloc)
    e_throw_BADOBJ(lv);

  if (lv->address == NULL && (lv->alloc || lv->count))
    e_throw_BADOBJ(lv);

  if (lv->count == lv->alloc) {
    size_t new_alloc = (lv->alloc + 1) << 2;
    void* new_addr = realloc(lv->address, new_alloc * s);
    if (new_addr == NULL)
      e_throw_MEMALL();

    lv->address = new_addr;
    lv->alloc = new_alloc;
  }

  void* address = lv->address + (lv->count * s);
  memcpy(address, e, s);
  lv->count++;
  return __OK__;
}

int lv_remove(listview_t* lv) {
  if (lv == NULL)
    e_throw_NULPTR(lv);

  if (lv->address == NULL)
    e_throw_BADOBJ(lv->address);

  if (lv->count == 0)
    e_throw_BADOBJ(lv->count);

  lv->count--;
  return __OK__;
}

int lv_resize(listview_t* lv, size_t n, size_t s) {
  if (lv == NULL)
    e_throw_NULPTR(lv);

  int is_zero = (n == 0 || s == 0);
  int is_null = (lv->address == NULL);

  // memory held by list is freed if zero bytes is requested
  if (is_zero && !is_null) {
    free(lv->address);
  }

  // list is put in a clean state if zero bytes is requested
  if (is_zero) {
    *lv = (listview_t){0};
    return __OK__;
  }

  // count is reduced to n if n is smaller than count
  if (n < lv->count) {
    lv->count = n;
  }

  // allocation
  size_t size = n * s;
  void* address = realloc(lv->address, size);
  if (address == NULL)
    e_throw_MEMALL();

  lv->address = address;
  lv->alloc = n;
  return __OK__;
}

int lv_clear(listview_t* lv, size_t s) {
  if (lv == NULL)
    e_throw_NULPTR(lv);

  lv->count = 0;
  return __OK__;
}
