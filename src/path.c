/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "path.h"

#define check_size(size, cleanup)                                              \
  if (size > PATH_MAX) {                                                       \
    cleanup;                                                                   \
    return pe_long;                                                            \
  }

#define check_errno(error, clean)                                              \
  errno = 0;                                                                   \
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

pe_t p_init(path_t *out, ps_t *chars) {
  assert(out != NULL);
  assert(chars != NULL);
  assert(chars->chars != NULL);
  check_size(chars->size, {});

  realpath(chars->chars, out->path);
  check_errno(pe_stdlib, {});

  size_t length = chars->size - 1;
  size_t depth = 0;
  if (p_isroot(out) == pe_root) {
    out->length = length;
    out->depth = depth;
    return pe_ok;
  }

  char *delim = out->path;
  while ((delim = strchr(delim, '/')) != NULL) {
    delim++;
    depth++;
  }

  out->length = length;
  out->depth = depth;
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
  *out = objstats.st_mode & S_IFMT;
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

  if (path->depth == 0) {
    return pe_root;
  }

  else if (path->depth == 1) {
    ps_t root = p_literal("/");
    return p_init(out, &root);
  }

  for (size_t i = path->length + 1; i > 0; i--) {
    if (path->path[i] == '/') {
      path->path[i] = '\0';
      ps_t parent = {.chars = path->path, .size = i + 1};
      pe_t error = p_init(out, &parent);
      path->path[i] = '/';
      return error;
    }
  }

  return pe_inv;
}

pe_t p_child(path_t *path, path_t *out, ps_t *chars) {
  assert(path != NULL);
  assert(out != NULL);
  assert(chars != NULL);
  assert(chars->chars != NULL);
  check_size(chars->size, {});

  char temp[PATH_MAX] = {0};
  strncpy(temp, path->path, PATH_MAX - 1);
  strncat(temp, "/", PATH_MAX - 1);
  strncat(temp, chars->chars, PATH_MAX - 1);
  ps_t child = p_decayed(temp);
  pe_t error = p_init(out, &child);
  return error;
}

pe_t p_isroot(path_t *path) {
  assert(path != NULL);
  return (strncmp(path->path, "/", PATH_MAX) == 0) ? pe_root : pe_ok;
}
