/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../app.h"
#include "../test.h"

int a_dummy_default_command(app_t *app, aa_t *args);
int a_dummy_other_command(app_t *app, aa_t *args);

/*******************************************************************************
 *                            Test data
 *******************************************************************************/

int a_defcmd_calls = 0;
int a_otrcmd_calls = 0;
ac_t a_commands[] = {
    a_command("default", "The default command", a_dummy_default_command),
    a_command("other", "An other command", a_dummy_other_command),
};

/*******************************************************************************
 *                           Dummy functions
 *******************************************************************************/

int a_dummy_default_command(app_t *app, aa_t *args) {
  if (app == NULL)
    return 1;

  if (args == NULL)
    return 2;

  if (args->argc != 1)
    return 3;

  if (args->argv == NULL)
    return 4;

  if (strcmp(args->argv[0], "argument0") != 0)
    return 5;

  a_defcmd_calls = 1;
  return 0;
}

int a_dummy_other_command(app_t *app, aa_t *args) {
  a_otrcmd_calls = 1;
  return 6;
}

/*******************************************************************************
 *                       Test utility functions
 *******************************************************************************/

void a_util_create_cmdbuf(app_t *app) {
  size_t num = 2;
  ac_t *buf = malloc(sizeof(ac_t) * num);
  buf[0] = a_commands[0];
  buf[1] = a_commands[1];
  app->cmdbuf = buf;
  app->cmdnum = num;
}

void a_util_create_argbuf(app_t *app, const str *args) {
  size_t alloc = 0;
  while (args[alloc] != NULL) {
    alloc++;
  }

  app->argbuf = malloc(sizeof(aa_t) * alloc);
  app->argalloc = alloc;

  for (size_t i = 0; i < alloc; i++) {
    app->argbuf[i] = args[i];
  }
}

app_t a_util_create_app(void) {
  str args[] = {"argument0", NULL};
  app_t app;
  app.closed = ae_opened;
  app.version = a_version(1, 2, 3);
  app.name[0] = '\0';
  app.input[0] = '\0';
  a_util_create_cmdbuf(&app);
  a_util_create_argbuf(&app, args);
  return app;
}

void a_util_close_app(app_t *app) {
  if (app->argbuf) {
    free(app->argbuf);
  }

  if (app->cmdbuf) {
    free(app->cmdbuf);
  }

  if (app->istream != stdin && app->istream != NULL) {
    fclose(app->istream);
  }

  memset(app, 0, sizeof(*app));
}

void a_util_input_from_file(app_t *app) {
  FILE *file = fopen("args.dat", "r");
  app->istream = file;
}

/*******************************************************************************
 *                           Test cases
 *******************************************************************************/

void a_test_init(void) {
  // arrange
  app_t app;
  ap_t params = {
      .name = "test app",
      .version = a_version(1, 0, 0),
      .commands = a_commands,
      .cmdnum = 2,
  };

  // act
  ae_t error = a_init(&app, &params);

  // assert
  size_t ansize = strnlen(app.name, sizeof(app.name)) + 1;
  size_t pnsize = strnlen(params.name, sizeof(params.name)) + 1;

  t_exp("%i", ae_ok, "%i", error, { a_util_close_app(&app); });
  t_nexp("%p", NULL, "%p", app.cmdbuf, { a_util_close_app(&app); });
  t_exp("%i", ae_opened, "%i", app.closed, { a_util_close_app(&app); });
  t_sexp(params.name, pnsize, app.name, ansize, { a_util_close_app(&app); });
  t_ok();

  a_util_close_app(&app);
}

void a_test_deinit(void) {
  // arrange
  app_t app = a_util_create_app();

  // act
  ae_t error = a_deinit(&app);

  // assert
  t_exp("%i", ae_ok, "%i", error, { free(app.cmdbuf); });
  t_exp("%p", NULL, "%p", app.cmdbuf, { a_util_close_app(&app); });
  t_exp("%p", NULL, "%p", app.argbuf, { a_util_close_app(&app); });
  t_exp("%zu", 0UL, "%zu", app.argalloc, { a_util_close_app(&app); });
  t_ok();
}

void a_test_dispatch(void) {
  // arrange
  app_t app = a_util_create_app();
  aa_t args = {.argc = app.argalloc, .argv = app.argbuf};

  // act
  ae_t error = a_dispatch(&app, "default", app.cmdbuf, app.cmdnum, &args);

  // assert
  t_exp("%i", ae_ok, "%i", error, { a_util_close_app(&app); });
  t_exp("%i", 1, "%i", a_defcmd_calls, { a_util_close_app(&app); });
  t_exp("%i", 0, "%i", app.result, { a_util_close_app(&app); });
  t_ok();

  a_util_close_app(&app);
}

void a_test_prompt(void) {
  // arrange
  app_t app = a_util_create_app();
  aa_t args;
  a_util_input_from_file(&app);

  // act
  ae_t error = a_prompt(&app, &args);

  // assert
  t_exp("%i", ae_ok, "%i", error, { a_util_close_app(&app); });
  t_exp("%zu", 2L, "%zu", args.argc, { a_util_close_app(&app); });

  size_t argsz0 = strlen(args.argv[0]) + 1;
  size_t argsz1 = strlen(args.argv[1]) + 1;
  size_t expsz0 = sizeof("other");
  size_t expsz1 = sizeof("arg1");

  t_sexp("other", expsz0, args.argv[0], argsz0, { a_util_close_app(&app); });
  t_sexp("arg1", expsz1, args.argv[1], argsz1, { a_util_close_app(&app); });
  t_ok();

  a_util_close_app(&app);
}

int main(int argc, char **argv) {
  a_test_init();
  a_test_deinit();
  a_test_dispatch();
  a_test_prompt();
  return 0;
}
