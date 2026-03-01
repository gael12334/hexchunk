/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "app.h"

/*******************************************************************************
 *                            Stream object definitions
 *******************************************************************************/

typedef struct {
  app_t app;
  stream_t stream;
} hex_t;
