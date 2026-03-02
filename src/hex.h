/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "app.h"
#include "path.h"

/*******************************************************************************
 *                            Hex object definitions
 *******************************************************************************/

/*
 * Alias of App's argument object type.
 */
typedef aa_t ha_t;

/*
 * Hex error codes
 */
typedef enum { he_ok, he_argc, he_state, he_number } he_t;

/*
 * Hex program states
 */
typedef enum { hs_ready, hs_occupied } hs_t;

/*
 * Hex object
 */
typedef struct {
  path_t path;
  stream_t stream;
  hs_t state;
} hex_t;

/*
 * Hex app object
 */
typedef struct {
  app_t app;
  hex_t hex;
} hexapp_t;

/*******************************************************************************
 *                              Hex functions
 *******************************************************************************/

/*
 * Open file
 */
int h_open(app_t *app, ha_t *args);

/*
 * Close file
 */
int h_close(app_t *app, ha_t *args);

/*
 * Move to file offset
 */
int h_move(app_t *app, ha_t *args);

/*
 * View bytes in hexadecimal viewer
 */
int h_view(app_t *app, ha_t *args);

/*
 * Save file offset
 */
int h_mark(app_t *app, ha_t *args);

/*
 * Remove file offset
 */
int h_unmark(app_t *app, ha_t *args);

/*
 * Go to saved offset
 */
int h_atmark(app_t *app, ha_t *args);

/*
 * Quit
 */
int h_quit(app_t *app, ha_t *args);

/*
 * Find a byte sequence
 */
int h_find(app_t *app, ha_t *args);

/*
 * Find images
 */
int h_findimg(app_t *app, ha_t *args);
