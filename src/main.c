#include "hex.h"

int main(int argc, char **argv) {
  hexapp_t app;
  aa_t aa;
  int err;

  err = h_init(&app);
  if (err)
    return err;

  while (a_closed(&app.app) != ae_closed) {
    err = a_prompt(&app.app, &aa);
    if (err)
      return err;

    err = a_dispatch(&app.app, aa.argv[0], app.app.cmdbuf, app.app.cmdnum, &aa);
    if (err)
      return err;
  }

  h_deinit(&app);
  return 0;
}
