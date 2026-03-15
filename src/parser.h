/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arrayview.h"
#include "error.h"
#include "listview.h"
#include "typedef.h"

/* delimiter -> spaces (space, tab, etc.)
 *
 * integer -> -1, 2, 30, 0
 * number -> -1, 2, 39.2, -2.4
 * keyword -> 1st=alpha, rest=alphanum
 * string -> ... until delim, or "..."
 * hexadecimal -> 8950474f
 */

/*
 * Constants
 */
enum {
  p_toknb = 64
};

/*
 * Command line interface parser function
 */
typedef int (*clipfn_t)(charview_t token, value_t* val, int* result);

/*
 * Command line interface parser token
 */
typedef struct {
  clipfn_t parser;
  charview_t token;
  charview_t input;
  value_t value;
} cliptok_t;

/*
 * Command line interface parser object
 */
typedef struct {
  cliptok_t tokens[64]; // list of tokens
  size_t toknsz;        // number of tokens
  uint64_t pmask;       // bitmask of wildcard tokens
  size_t pnmbr;         // number of wildcard tokens
} clip_t;

/*******************************************************************************
 *                             Parser functions
 *******************************************************************************/

int p_init(clip_t* clip, charview_t format);
int p_deinit(clip_t* clip);
int p_match(clip_t* clip, charview_t input, int* result);
int p_param(clip_t* clip, size_t pndx, size_t* real_ndx);
