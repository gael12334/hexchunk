/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "routes.h"

void r_init(ac_t **routes, size_t *number) {
  assert(routes != NULL);
  assert(number != NULL);

  static ac_t hex_routes[] = {
      a_command("open", "open a binary stream", r_open),
      a_command("close", "close the binary stream", r_close),
      a_command("move", "move stream position", r_move),
      a_command("view", "view stream bytes in hexadecimal viewer", r_view),
      a_command("find", "find pattern from current stream position", r_find),
      a_command("where", "display the current stream index", r_where),
      a_command("dump", "dump stream data in a file", r_dump)};

  *routes = hex_routes;
  *number = sizeof(hex_routes) / sizeof(hex_routes[0]);
}

void r_deinit(ac_t **routes, size_t *number) {
  assert(routes != NULL);
  assert(number != NULL);

  *routes = NULL;
  *number = 0;
}

int r_open(void *app, aa_t *args) {
  assert(app != NULL);
  assert(args != NULL);

  hexapp_t *hexapp = (hexapp_t *)app;
}

int r_close(void *app, aa_t *args);

int r_move(void *app, aa_t *args);

int r_view(void *app, aa_t *args);

int r_find(void *app, aa_t *args);

int r_where(void *app, aa_t *args);

int r_dump(void *app, aa_t *args);
