/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#include "app.h"
#include "hex.h"

void r_init(ac_t **routes, size_t *number);

void r_deinit(ac_t **routes, size_t *number);

int r_open(void *app, aa_t *args);

int r_close(void *app, aa_t *args);

int r_move(void *app, aa_t *args);

int r_view(void *app, aa_t *args);

int r_find(void *app, aa_t *args);

int r_where(void *app, aa_t *args);

int r_dump(void *app, aa_t *args);
