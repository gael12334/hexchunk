/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#define u32_wrap(x3, x2, x1, x0) (x3 << 24) | (x2 << 16) | (x1 << 8) | (x0 << 0)
typedef const char *cstr;
typedef char *str;
