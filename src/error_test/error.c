/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../error.h"
#include "../test.h"

/*******************************************************************************
 *                              utility functions
 *******************************************************************************/

int e_util_throwing_function(void) {
  e_throw(1, "Test %s", "Test");
}

int e_util_successful_function(void) {
  return 0;
}

/*******************************************************************************
 *                                  tests
 *******************************************************************************/

void e_test_throw_report_pushtrace(void) {
  // arrange

  // act
  int e = e_util_throwing_function();

  // assert
  t_exp("%i", 1, "%i", e, {});
  t_ok();
}

void e_test_try_catch_lastcode_calltrace(void) {
  // arrange
  e_clear();
  int caught = 0;
  int handled = 0;
  calltrace_t* trace = NULL;
  const char str0[] = "Test Test";
  const char str1[] = "Fault caught !";
  size_t str0_sz = sizeof(str0);
  size_t str1_sz = sizeof(str1);

  // act
  e_try(e_util_throwing_function(), { caught = 1; });
  e_catch(calltrace_t * calltrace, {
    handled = 1;
    trace = calltrace;
  });

  // assert
  const errmsg_t* errmsg = &trace->list[0].errmsg;
  const size_t errmsg_sz = sizeof(errmsg->msg);
  const size_t lenmsg = strnlen(errmsg->msg, errmsg_sz) + 1;

  t_exp("%i", 1, "%i", caught, {});
  t_exp("%i", 1, "%i", handled, {});
  t_nexp("%p", NULL, "%p", trace, {});
  t_exp("%i", 2, "%lu", trace->count, {});
  t_sexp(str0, str0_sz, errmsg->msg, lenmsg, {});
  t_ok();
}

void e_test_try_catch_lastcode_calltrace_nothrow(void) {
  // arrange
  e_clear();
  int caught = 0;
  int handled = 0;
  calltrace_t* trace = NULL;

  // act
  e_try(e_util_successful_function(), { caught = 1; });
  e_catch(calltrace_t * calltrace, {
    handled = 1;
    trace = calltrace;
  });

  // assert
  t_exp("%i", 0, "%i", caught, {});
  t_exp("%i", 0, "%i", handled, {});
  t_exp("%p", NULL, "%p", trace, {});
  t_ok();
}

int main() {
  e_test_throw_report_pushtrace();
  e_test_try_catch_lastcode_calltrace();
  e_test_try_catch_lastcode_calltrace_nothrow();
  return 0;
}
