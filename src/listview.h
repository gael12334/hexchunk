/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "error.h"
#include "typedef.h"

#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
 *                                listview types
 *******************************************************************************/

typedef struct listview {
  void* address;
  size_t alloc;
  size_t count;
} listview_t;

#define listviewof_t(T)                                                                  \
  struct {                                                                               \
    T* address;                                                                          \
    size_t alloc;                                                                        \
    size_t count;                                                                        \
  }

/*******************************************************************************
 *                              listview functions
 *******************************************************************************/

/*
 * Append an element to a list view
 */
int lv_append(listview_t* lv, void* e, size_t s);
#define lvof_append(lv, e) lv_append((listview_t*)(lv), e, sizeof(*(lv)->address))

/*
 * Remove last element from list view
 */
int lv_remove(listview_t* lv);
#define lvof_remove(lv) lv_remove((listview_t*)(lv))

/*
 * Resize a list view
 * - n : new number of elements
 * - s : size in bytes of one element
 */
int lv_resize(listview_t* lv, size_t n, size_t s);
#define lvof_resize(lv, n) lv_resize((listview_t*)(lv), n, sizeof(*(lv)->address))

/*
 * Clear a list view
 */
int lv_clear(listview_t* lv, size_t s);
#define lvof_clear(lv) lv_clear((listview_t*)(lv), sizeof(*(lv)->address))

/*
 * For each element in list view
 */
#define lvof_foreach(var, lv)                                                            \
  for (var = (lv)->address; var != (lv)->address + (lv)->count; var++)

/*
 * Index of element in list view
 */
#define lvof_indexof(lv, var) (var - (lv)->address)
