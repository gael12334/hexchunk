/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "typedef.h"

/*******************************************************************************
 *                         Path object definitions
 *******************************************************************************/

/*
 * Path error codes
 */

typedef enum { pe_ok, pe_long, pe_stdlib, pe_sysstat, pe_root, pe_end } pe_t;

/*
 * Path's pointed object type (from <sys/stat.h>)
 */
typedef enum {
  po_file = S_IFREG,
  po_dir = S_IFDIR,
  po_block = S_IFBLK,
  po_char = S_IFCHR,
  po_link = S_IFLNK,
  po_fifo = S_IFIFO,
  po_socket = S_IFSOCK
} po_t;

/*
 * Path object
 */
typedef struct {
  char path[PATH_MAX];
  size_t length;
  size_t depth;
} path_t;

/*******************************************************************************
 *                            Path functions
 *******************************************************************************/

/*
 * Init instance of path
 */
pe_t p_init(path_t *out, cstr path, size_t length);

/*
 * Deinit instance of path
 */
pe_t p_deinit(path_t *path);

/*
 * Type of object pointed by path
 */
pe_t p_typeof(path_t *path, po_t *out);

/*
 * Length of path
 */
pe_t p_length(path_t *path, size_t *out);

/*
 * Logical depth of path
 */
pe_t p_depth(path_t *path, size_t *out);

/*
 * Null-terminated string of path
 */
pe_t p_string(path_t *path, char **const out);

/*
 * Path of the pointed object's parent
 */
pe_t p_parent(path_t *path, path_t *out);

/*
 * Path of one of the pointed object's children
 */
pe_t p_child(path_t *path, path_t *out, cstr name, size_t size);

/*
 * Tell if path points to root directory
 */
pe_t p_isroot(path_t *path);
