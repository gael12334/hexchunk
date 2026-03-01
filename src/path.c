/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "path.h"

#define check_length(length, cleanup)                                          \
  if (length > PATH_MAX) {                                                     \
    cleanup;                                                                   \
    return pe_long;                                                            \
  }

#define check_errno(error, clean)                                              \
  if (errno) {                                                                 \
    clean;                                                                     \
    return error;                                                              \
  }

#define check_pe(pe, clean)                                                    \
  if (pe != pe_ok) {                                                           \
    clean;                                                                     \
    return pe;                                                                 \
  }

/*******************************************************************************
 *                            Path functions
 *******************************************************************************/

pe_t p_init(path_t *out, cstr path, size_t length) {
  assert(out != NULL);
  assert(path != NULL);
  check_length(length, {});

  realpath(path, out->path);
  check_errno(pe_stdlib, {});
  out->length = strnlen(out->path, sizeof(out->path));

  char *delim = out->path;
  while ((delim = strchr(delim, '/')) != NULL) {
    out->depth++;
  }

  return pe_ok;
}

pe_t p_deinit(path_t *path) {
  assert(path != NULL);
  memset(path, 0, sizeof(*path));
  return pe_ok;
}

pe_t p_typeof(path_t *path, po_t *out) {
  assert(path != NULL);
  assert(out != NULL);
  struct stat objstats;
  stat(path->path, &objstats);
  check_errno(pe_sysstat, {});
  *out = objstats.st_mode;
  return pe_ok;
}

pe_t p_length(path_t *path, size_t *out) {
  assert(path != NULL);
  assert(out != NULL);
  *out = path->length;
  return pe_ok;
}

pe_t p_depth(path_t *path, size_t *out) {
  assert(path != NULL);
  assert(out != NULL);
  *out = path->depth;
  return pe_ok;
}

pe_t p_string(path_t *path, char **const out) {
  assert(path != NULL);
  assert(out != NULL);
  *out = path->path;
  return pe_ok;
}

pe_t p_parent(path_t *path, path_t *out) {
  assert(path != NULL);
  assert(out != NULL);

  pe_t err = p_init(out, path->path, path->length);
  check_pe(err, {});

  char *delim = strrchr(out->path, '/');
  if (delim == out->path)
    return pe_root;

  *delim = '\0';
  out->depth--;
  out->length = delim - &out->path[0];
  return pe_ok;
}

pe_t p_child(path_t *path, path_t *out, cstr name, size_t size) {
  assert(path != NULL);
  assert(out != NULL);
  assert(name != NULL);

  char temp[PATH_MAX];
  if (path->length + size + 1 > PATH_MAX - 1)
    return pe_long;

  size_t i = 0;
  while (i < path->length) {
    temp[i] = path->path[i];
    i++;
  }

  temp[i++] = '/';

  size_t end = i + size;
  while (i < end) {
    temp[i] = name[i];
    i++;
  }

  temp[i] = '\0';

  pe_t err = p_init(out, temp, i);
  return err;
}
