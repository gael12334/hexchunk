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

#define check_closed(a, clean)                                                 \
  if (a->closed != ae_opened) {                                                \
    clean;                                                                     \
    return ae_closed;                                                          \
  }

/*******************************************************************************
 *                            Functions
 *******************************************************************************/

ae_t a_init(app_t *out, ap_t *params) {
  assert(out != NULL);
  assert(params != NULL);
  assert(params->commands != NULL);

  out->closed = ae_opened;
  out->version = params->version;
  out->cmdnum = params->cmdnum;

  out->cmdbuf = malloc(sizeof(ac_t) * params->cmdnum);
  assert(out->cmdbuf != NULL);

  ac_t *end = params->commands + params->cmdnum;
  ac_t *list = params->commands;

  for (ac_t *ac = list; ac != end; ac++) {
    check_null(ac->delegate, { free(out->cmdbuf); });
    check_null(ac->name, { free(out->cmdbuf); });
    out->cmdbuf[ac - list] = *ac;
  }

  out->istream = stdin;
  out->argalloc = 0;
  out->argbuf = NULL;
  memset(out->input, 0, sizeof(out->input));
  strncpy(out->name, params->name, sizeof(out->name));
  return ae_ok;
}

ae_t a_deinit(app_t *a) {
  assert(a != NULL);
  assert(a->cmdbuf != NULL);
  free(a->cmdbuf);
  if (a->argbuf) {
    free(a->argbuf);
  }
  memset(a, 0, sizeof(*a));
  return ae_ok;
}

void a_dispatch(app_t *a, cstr name, ac_t *cmds, size_t cmdnum, aa_t *args) {
  assert(a != NULL);
  assert(cmds != NULL);
  assert(args != NULL);
  for (ac_t *ac = cmds, *end = cmds + cmdnum; ac != end; ac++) {
    if (strncmp(name, ac->name, sizeof(a->input)) == 0) {
      a->result = ac->delegate(a, args);
      return;
    }
  }

  printf("Error: '%s' unknown command\n", name);
}

ae_t a_prompt(app_t *a, aa_t *out) {
  assert(a != NULL);
  assert(out != NULL);
  assert(a->istream != NULL);
  check_closed(a, {});

  printf("%s > ", a->name);
  fgets(a->input, sizeof(a->input) - 1, a->istream);
  size_t newline = strcspn(a->input, "\n\r");
  a->input[newline] = '\0';

  size_t num = 0;
  str token = strtok(a->input, " ");
  while (token != NULL) {
    if (num >= a->argalloc) {
      a->argalloc++;
      a->argalloc *= 2;
      a->argbuf = realloc(a->argbuf, a->argalloc);
      assert(a->argbuf != NULL);
    }

    a->argbuf[num] = token;
    token = strtok(NULL, " ");
    num++;
  }

  out->argv = a->argbuf;
  out->argc = num;
  return ae_ok;
}

ae_t a_closed(app_t *a) {
  assert(a != NULL);
  return a->closed;
}

ae_t a_arg2long(cstr arg, long *out) {
  assert(arg != NULL);
  assert(out != NULL);

  char *end;
  *out = strtol(arg, &end, 10);

  int zero_digits = (end == arg);
  int non_digits = (*end != '\0');
  int errored = (errno != 0);

  if (zero_digits || non_digits)
    return ae_number;

  else if (errored)
    return ae_stdlib;

  else
    return ae_ok;
}

int a_help(struct app *a, aa_t *args) {
  assert(a != NULL);
  assert(args != NULL);

  printf("%s\n", a->name);
  for (size_t i = 0; i < a->cmdnum; i++) {
    printf("%s: %s\n", a->cmdbuf[i].name, a->cmdbuf[i].description);
  }

  return ae_ok;
}

static at_t *a_parseformat(str start, cstr end, size_t i, size_t *num) {
  // assertion
  assert(start != NULL);
  assert(num != NULL);

  // check if end reached
  if (start == end) {
    *num = i;
    return NULL;
  }

  // length of token
  size_t n = 0;
  while (start[n] != *end && start[n] != ' ')
    n++;

  // replace space by NUL
  size_t b = n;
  while (start[b] != *end && start[b] == ' ')
    start[b++] = '\0';

  // next iteration
  at_t *result = a_parseformat(start + b, end, i + 1, num);

  // if num is also zero, it means there was an error.
  if (result == NULL && *num == 0)
    return NULL;

  // tokens
  at_t *tokens = (result) ? result : malloc(sizeof(at_t) * (i + 1));
  assert(tokens != NULL);

  // return null if token only contains '%'
  if (start[0] == '%' && n == 1) {
    if (tokens != NULL)
      free(tokens);
    return NULL;
  }

  // %s -> variable 'name' of type 'string'
  // %x -> variable 'name' of type 'hex'
  // %i -> variable 'name' of type 'integer'
  // %f -> variable 'name' of type 'decimal'
  an_t type = an_identif;
  size_t size = 0;
  if (start[0] == '%' && n >= 2) {
    switch (start[1]) {
    case an_integer:
      type = an_integer;
      size = sizeof(long);
      break;
    case an_hexadec:
      type = an_hexadec;
      size = sizeof(long);
      break;
    case an_decimal:
      type = an_decimal;
      size = sizeof(double);
      break;
    case an_charseq:
      type = an_charseq;
      size = 0;
      break;
    default:
      if (tokens != NULL)
        free(tokens);
      return NULL;
    }
  }

  // allocate data
  void *data = malloc(size);
  assert(data != NULL);
  tokens[i] = (at_t){
      .data = data,
      .size = size,
      .text = start,
      .type = type,
  };

  return tokens;
}

int a_addroute(app_t *a, str format, size_t length, ad_t function) {
  assert(a != NULL);
  assert(format != NULL);
  assert(function != NULL);

  size_t num = 0;
  at_t *tokens = a_parseformat(format, format + length, 0, &num);

  if (tokens == NULL && num == 0) {
    return ae_null;
  }
}
