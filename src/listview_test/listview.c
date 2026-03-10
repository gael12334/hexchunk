/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../listview.h"
#include "../test.h"

void lv_test_resize(void) {
  // arrange
  e_clear();
  size_t n = 10;
  listviewof_t(int) list = {0};

  // act
  e_trycatch(lvof_resize(&list, n), { e_printtrace(stdout); });

  // assert
  free(list.address);
  t_nexp("%p", NULL, "%p", list.address, {});
  t_exp("%i", __OK__, "%i", e_lastcode(), {});
  t_exp("%zu", n, "%lu", list.alloc, {});
  t_exp("%i", 0, "%lu", list.count, {});
  t_ok();
}

void lv_test_resize_free(void) {
  // arrange
  e_clear();
  listviewof_t(int) list = {.alloc = 3, .count = 0};
  list.address = malloc(sizeof(int) * list.alloc);

  // act
  e_trycatch(lvof_resize(&list, 0), { e_printtrace(stdout); });

  // assert
  t_exp("%p", NULL, "%p", list.address, { free(list.address); });
  t_exp("%i", __OK__, "%i", e_lastcode(), {});
  t_exp("%i", 0, "%lu", list.alloc, {});
  t_exp("%i", 0, "%lu", list.count, {});
  t_ok();
}

void lv_test_resize_less_than_count(void) {
  // arrange
  e_clear();
  size_t alloc2 = 3;
  listviewof_t(int) list = {.alloc = 10, .count = 5};
  list.address = malloc(sizeof(int) * list.alloc);

  // act
  e_trycatch(lvof_resize(&list, alloc2), { e_printtrace(stdout); });

  // assert
  if (list.address)
    free(list.address);

  t_nexp("%p", NULL, "%p", list.address, {});
  t_exp("%i", __OK__, "%i", e_lastcode(), {});
  t_exp("%zu", alloc2, "%lu", list.alloc, {});
  t_exp("%zu", alloc2, "%lu", list.count, {});
  t_ok();
}

void lv_test_append_badobj(void) {
  // arrange
  e_clear();
  listviewof_t(int) list = {.address = NULL, .alloc = 4};

  // act
  int value = 3;
  e_trycatch(lvof_append(&list, &value), { e_printtrace(stdout); });

  // assert

  t_exp("%p", NULL, "%p", list.address, {});
  t_exp("%i", EBADOBJ, "%i", e_lastcode(), {});
  t_ok();
}

void lv_test_append_empty(void) {
  // arrange
  e_clear();
  listviewof_t(int) list = {0};

  // act
  int value = 3;
  e_trycatch(lvof_append(&list, &value), { e_printtrace(stdout); });

  // assert
  int result = 0;
  if (list.address && list.count >= 1) {
    result = list.address[0];
    free(list.address);
  }

  t_nexp("%p", NULL, "%p", list.address, {});
  t_exp("%i", __OK__, "%i", e_lastcode(), {});
  t_exp("%i", 1, "%lu", list.count, {});
  t_exp("%i", value, "%i", result, {});
  t_ok();
}

void lv_test_remove(void) {
  // arrange
  listviewof_t(int) list = {.alloc = 3, .count = 2};
  list.address = malloc(sizeof(int) * list.alloc);
  void* address = list.address;

  // act
  e_trycatch(lvof_remove(&list), { e_printtrace(stdout); });

  // assert
  if (list.address)
    free(list.address);

  t_exp("%p", address, "%p", list.address, {});
  t_exp("%i", 3, "%zu", list.alloc, {});
  t_exp("%i", 1, "%zu", list.count, {});
  t_ok();
}

void lv_test_remove_empty(void) {
  // arrange
  listviewof_t(int) list = {.address = NULL, .alloc = 3, .count = 2};

  // act
  e_trycatch(lvof_remove(&list), { e_printtrace(stdout); });

  // assert
  t_exp("%p", NULL, "%p", list.address, {});
  t_exp("%i", EBADOBJ, "%i", e_lastcode(), {});
  t_ok();
}

int main() {
  lv_test_resize();
  lv_test_resize_free();
  lv_test_resize_less_than_count();
  lv_test_append_badobj();
  lv_test_append_empty();
  lv_test_remove();
  lv_test_remove_empty();
  return 0;
}
