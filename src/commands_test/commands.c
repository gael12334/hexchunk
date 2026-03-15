/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../commands.h"
#include "../test.h"

/*******************************************************************************
 *                       Test utility functions
 *******************************************************************************/

int c_util_dumcmd(void* extra, size_t argc, value_t* argv) {
  return __OK__;
}

/*******************************************************************************
 *                           Test cases
 *******************************************************************************/

void c_test_init(void) {
  // arrange
  cmdmgr_t mgr = {0};
  int errcaught = 0;

  // act
  e_trycatch(c_init(&mgr), errcaught = 1);

  // assert
  if (mgr.commands.address)
    free(mgr.commands.address);

  t_exp("%i", 0, "%i", errcaught, {});
  t_exp("%i", c_default_alloc, "%zu", mgr.commands.alloc, {});
  t_ok();
}

void c_test_deinit(void) {
  // arrange
  cmdmgr_t mgr = {.commands.address = malloc(sizeof(cmd_t) * 10),
                  .commands.alloc = 10,
                  .commands.count = 0};
  int errcaught = 0;

  // act
  e_trycatch(c_deinit(&mgr), errcaught = 1);

  // assert
  if (mgr.commands.address)
    free(mgr.commands.address);

  t_exp("%i", 0, "%i", errcaught, {});
  t_exp("%i", 0, "%zu", mgr.commands.alloc, {});
  t_ok();
}

void c_test_register(void) {
  // arrange
  int errthrown = 0;
  cmdmgr_t mgr = {0};
  c_init(&mgr);

  // act
  charview_t pattern = cvtext("dum *");
  vaargs_t vaargs = valist(catype_t, CARGINT);
  e_try(c_register(&mgr, pattern, vaargs, c_util_dumcmd), { errthrown = 1; });

  // assert
  e_catch(calltrace_t * c, { e_printtrace(stdout); });

  cmd_t cmd = {0};
  if (mgr.commands.address) {
    cmd = mgr.commands.address[0];
    free(mgr.commands.address);
  }

  const char str[] = "dum *";
  size_t strsz = sizeof(str);

  t_exp("%i", 0, "%i", errthrown, {});
  t_exp("%p", c_util_dumcmd, "%p", cmd.handler, {});
  t_exp("%i", 1, "%zu", mgr.commands.count, {});
  t_sexp(str, strsz, cmd.pattern.address, cmd.pattern.count, {});
  t_ok();
}

void c_test_register_badobj(void) {
  // arrange
  int errthrown = 0;
  cmdmgr_t mgr = {0};
  c_init(&mgr);

  // act
  charview_t pattern = {.address = NULL, .count = 3};
  vaargs_t vaargs = valist(catype_t, CARGINT);
  e_try(c_register(&mgr, pattern, vaargs, c_util_dumcmd), { errthrown = 1; });

  // assert
  e_catch(calltrace_t * c, { e_printtrace(stdout); });

  cmd_t cmd = {0};
  if (mgr.commands.address) {
    cmd = mgr.commands.address[0];
    free(mgr.commands.address);
  }

  t_exp("%i", 1, "%i", errthrown, {});
  t_ok();
}

int main() {
  c_test_init();
  c_test_deinit();
  c_test_register();
  c_test_register_badobj();
  return 0;
}
