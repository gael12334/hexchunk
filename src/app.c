/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "app.h"

/*******************************************************************************
 *                            Checks
 *******************************************************************************/

#define check_null(ptr, clean)                                                 \
  if (ptr == NULL) {                                                           \
    clean;                                                                     \
    return ae_null;                                                            \
  }

#define check_ae(ae, clean)                                                    \
  if (ae != ae_ok) {                                                           \
    clean;                                                                     \
    return ae;                                                                 \
  }

/*******************************************************************************
 *                            Functions
 *******************************************************************************/
ae_t a_init(app_t *out, cstr name, av_t *version, ac_t *list, size_t num) {
  assert(out != NULL);
  assert(name != NULL);
  assert(version != NULL);
  assert(list != NULL);

  out->closed = ae_closed;
  out->version = *version;
  out->num = num;

  out->list = malloc(sizeof(ac_t) * num);
  assert(out->list != NULL);
  for (ac_t *ac = list, *end = list + num; ac != end; ac++) {
    check_null(ac->delegate, { free(out->list); });
    check_null(ac->name, { free(out->list); });
    out->list[ac - list] = *ac;
  }

  memset(out->input, 0, sizeof(out->input));

  strncpy(out->name, name, sizeof(out->name));
  out->namelen = strnlen(out->name, sizeof(out->namelen - 2));
  out->name[out->namelen - 1] = '\0';

  return ae_ok;
}

ae_t a_deinit(app_t *a) {
  assert(a != NULL);
  assert(a->list != NULL);
  free(a->list);
  memset(a, 0, sizeof(*a));
  return ae_ok;
}

ae_t a_dispatch(app_t *a, cstr name, ac_t *list, size_t num, aa_t *args) {
  assert(a != NULL);
  assert(list != NULL);
  assert(args != NULL);
  for (ac_t *ac = list, *end = list + num; ac != end; ac++) {
    if (strncmp(name, list->name, sizeof(a->input)) == 0) {
      return list->delegate(a, args);
    }
  }

  puts("Error: '%s' unknown command");
  return ae_unknown;
}

ae_t a_run(app_t *a) {
  assert(a != NULL);
  assert(a->list != NULL);
  assert(a->name != NULL);
  assert(a->closed == ae_closed);

  aa_t args = {.argc = 0, .argv = NULL};
  str *toklist = NULL;
  str token = NULL;
  size_t alloc = 0;
  size_t num = 0;

  a->closed = ae_open;
  while (a->closed == ae_open) {
    fwrite(a->name, a->namelen, 1, stdout);
    putc('>', stdout);
    fgets(a->input, sizeof(a->input) - 1, stdin);
    a->input[strcspn(a->input, "\n\r")] = '\0';

    num = 0;
    token = strtok(a->input, " ");
    while (token != NULL) {
      if (num == alloc) {
        alloc = (alloc + 1) * 2;
        toklist = realloc(toklist, sizeof(*toklist) * alloc);
        assert(toklist != NULL);
      }

      toklist[num] = token;
      token = strtok(NULL, " ");
      num++;
    }

    args.argv = toklist;
    args.argc = num;

    if (num > 0) {
      ae_t err = a_dispatch(a, toklist[0], a->list, a->num, &args);
      check_ae(err, { free(toklist); });
    }
  }

  if (toklist == NULL) {
    free(toklist);
  }
  return ae_ok;
}

ae_t a_closed(app_t *a) { return ae_ok; }
