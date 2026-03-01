/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../path.h"
#include "../test.h"

/*******************************************************************************
 *                            Test data
 *******************************************************************************/
char p_buf[] = "/home";
char p_tmp[] = "tmp.txt";
char p_tmpfull[PATH_MAX] = {0};

/*******************************************************************************
 *                       Test utility functions
 *******************************************************************************/

void p_util_create_tmpfile(void) {
  realpath(".", p_tmpfull);
  strncat(p_tmpfull, "/", PATH_MAX - 1);
  strncat(p_tmpfull, p_tmp, PATH_MAX - 1);
  FILE *file = fopen(p_tmpfull, "w");
  fwrite(p_buf, sizeof(p_buf), 1, file);
  fclose(file);
}

/*******************************************************************************
 *                           Test cases
 *******************************************************************************/

void p_test_init(void) {
  // arrange
  ps_t ps = p_literal(p_buf);
  path_t path;

  // act
  pe_t error = p_init(&path, &ps);

  // assert
  t_exp("%i", pe_ok, "%i", error, {});
  t_exp("%lu", 1L, "%lu", path.depth, {});
  t_exp("%lu", ps.size - 1, "%lu", path.length, {});
  t_ok();
}

void p_test_typeof(void) {
  // arrange
  po_t type;
  ps_t ps = p_literal(p_buf);
  path_t path;
  p_init(&path, &ps);

  // act
  pe_t error = p_typeof(&path, &type);

  // assert
  t_exp("%i", pe_ok, "%i", error, {});
  t_exp("%i", po_dir, "%i", type, {});
  t_ok();
}

void p_test_parent(void) {
  // arrange
  path_t r = {0};
  path_t path;
  ps_t ps = p_literal(p_buf);
  ps_t x = p_literal("/");
  p_init(&path, &ps);

  // act
  pe_t error = p_parent(&path, &r);

  // assert
  t_exp("%i", pe_ok, "%i", error, {});
  t_exp("%lu", 0UL, "%lu", r.depth, {});
  t_sexp(x.chars, x.size, r.path, r.length + 1, {});
  t_ok();
}

void p_test_child(void) {
  // arrange
  p_util_create_tmpfile();
  ps_t x = p_decayed(p_tmpfull);
  ps_t ps = p_literal(".");
  ps_t c = p_literal(p_tmp);
  path_t path;
  path_t r;
  p_init(&path, &ps);

  // act
  pe_t error = p_child(&path, &r, &c);

  // assert
  t_exp("%i", pe_ok, "%i", error, { remove(p_tmpfull); });
  t_sexp(x.chars, x.size, r.path, r.length + 1, { remove(p_tmpfull); });
  t_ok();
  remove(p_tmpfull);
}

int main(int argc, char **argv) {
  p_test_init();
  p_test_typeof();
  p_test_parent();
  p_test_child();
  return 0;
}
