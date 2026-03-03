/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../hex.h"
#include "../test.h"

/*******************************************************************************
 *                            Test data
 *******************************************************************************/

/*******************************************************************************
 *                       Test utility functions
 *******************************************************************************/

hexapp_t h_util_create_app(ad_t fn) {
  hexapp_t app;
  static ac_t commands[1];
  commands[0] = a_command("test", "test", fn);
  ap_t ap = {
      .cmdnum = 1,
      .commands = commands,
      .name = "testapp",
      .version = a_version(0, 0, 0),
  };

  a_init(&app.app, &ap);
  app.hex.state = hs_ready;
  return app;
}

hexapp_t h_util_create_app_open_file(ad_t fn) {
  stream_t stream;
  hexapp_t app;
  ps_t ps;
  int err;

  app = h_util_create_app(fn);
  ps = p_literal("dump.sample");
  err = s_openfile(&app.hex.stream, ps.chars, sm_binary_read);
  assert(err == se_ok);
  p_init(&app.hex.path, &ps);
  app.hex.state = hs_occupied;

  return app;
}

void h_util_destroy_app(hexapp_t *app) {
  a_deinit(&app->app);
  s_close(&app->hex.stream);
}

/*******************************************************************************
 *                           Test cases
 *******************************************************************************/

void h_test_open(void) {
  // arrange
  hexapp_t app = h_util_create_app(h_open);
  str args[] = {"test", "dump.sample"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  hs_t state = app.hex.state;
  int result = app.app.result;
  h_util_destroy_app(&app);

  t_exp("%i", he_ok, "%i", result, {});
  t_exp("%i", hs_occupied, "%i", state, {});
  t_ok();
}

void h_test_open_failed(void) {
  // arrange
  hexapp_t app = h_util_create_app(h_open);
  str args[] = {"test", "elephant.elf"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  h_util_destroy_app(&app);

  t_exp("%i", se_null, "%i", result, {});
  t_ok();
}

void h_test_close(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_close);
  str args[] = {"test"};
  aa_t aa = {.argc = 1, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;

  t_exp("%i", he_ok, "%i", result, { h_util_destroy_app(&app); });
  t_exp("%i", hs_ready, "%i", app.hex.state, { h_util_destroy_app(&app); });
  t_ok();
}

void h_test_move(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_move);
  str args[] = {"test", "200"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  long pos = ftell(app.hex.stream.handle);
  h_util_destroy_app(&app);
  t_exp("%i", he_ok, "%i", result, {});
  t_exp("%li", 200L, "%li", pos, {});
  t_ok();
}

void h_test_move_somehow_works(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_move);
  str args[] = {"test", "-3"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  long pos = ftell(app.hex.stream.handle);
  h_util_destroy_app(&app);
  t_exp("%i", he_ok, "%i", result, {});
  t_exp("%li", 0L, "%li", pos, {});
  t_ok();
}

void h_test_view(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_view);
  str args[] = {"test", "200"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  h_util_destroy_app(&app);
  t_exp("%i", he_ok, "%i", result, {});
  t_ok();
}

void h_test_view_failed(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_view);
  str args[] = {"test", "-3"};
  aa_t aa = {.argc = 2, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  h_util_destroy_app(&app);
  t_exp("%i", he_size, "%i", result, {});
  t_ok();
}

void h_test_find(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_find);
  str args[] = {"test", "ELF", "200"};
  aa_t aa = {.argc = 3, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  h_util_destroy_app(&app);
  t_exp("%i", he_ok, "%i", result, {});
  t_ok();
}

void h_test_findx(void) {
  // arrange
  hexapp_t app = h_util_create_app_open_file(h_findx);
  str args[] = {"test", "53E57464", "1000"};
  aa_t aa = {.argc = 3, .argv = args};

  // act
  a_dispatch(&app.app, "test", app.app.cmdbuf, app.app.cmdnum, &aa);

  // assert
  int result = app.app.result;
  h_util_destroy_app(&app);
  t_exp("%i", he_ok, "%i", result, {});
  t_ok();
}

int main() {
  h_test_open();
  h_test_open_failed();
  h_test_close();
  h_test_move();
  h_test_move_somehow_works();
  h_test_view();
  h_test_view_failed();
  h_test_find();
  h_test_findx();
  return 0;
}
