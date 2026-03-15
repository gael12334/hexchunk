/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../arrayview.h"
#include "../test.h"

void av_test_alloc(void) {
  // arrange
  arrayviewof_t(int) intarr = {0};
  int errcaught = 0;

  // act
  e_trycatch(avof_alloc(&intarr, 10), errcaught = 1);

  // assert
  if (intarr.address)
    free(intarr.address);

  t_nexp("%p", NULL, "%p", intarr.address, {});
  t_exp("%i", 10, "%zu", intarr.count, {});
  t_exp("%i", 0, "%i", errcaught, {});
  t_ok();
}

void av_test_free(void) {
  // arrange
  arrayviewof_t(int) intarr = {.address = malloc(sizeof(int) * 10), .count = 10};
  int errcaught = 0;

  // act
  e_trycatch(avof_free(&intarr), errcaught = 1);

  // assert
  if (intarr.address)
    free(intarr.address);

  t_exp("%p", NULL, "%p", intarr.address, {});
  t_nexp("%i", 10, "%zu", intarr.count, {});
  t_exp("%i", 0, "%i", errcaught, {});
  t_ok();
}

void av_test_free_badobj(void) {
  // arrange
  arrayviewof_t(int) intarr = {.address = NULL, .count = 10};
  int errcaught = 0;

  // act
  e_trycatch(avof_free(&intarr), errcaught = 1);

  // assert
  t_exp("%i", 1, "%i", errcaught, {});
  t_exp("%i", 10, "%zu", intarr.count, {});
  t_exp("%p", NULL, "%p", intarr.address, {});
  t_ok();
}

void av_test_foreach(void) {
  // arrange
  arrayviewof_t(int) intarr = {.address = malloc(sizeof(int) * 10), .count = 10};
  int count = 0;

  // act
  avof_foreach(int* i, &intarr, { count++; });

  // assert
  if (intarr.address)
    free(intarr.address);
  t_exp("%i", 10, "%i", count, {});
  t_ok();
}

int main() {
  av_test_alloc();
  av_test_free();
  av_test_free_badobj();
  av_test_foreach();
  return 0;
}
