/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#include <ctype.h>

// #include "app.h"
#include "path.h"
#include "stream.h"

/*******************************************************************************
 *                            Hex object definitions
 *******************************************************************************/

/*
 * 16 bytes string for a row of 16 consecutive bytes
 */
typedef int8_t h16_t[16];

/*
 * 3 bytes string for 1 hex formatted byte
 */
typedef int8_t h3_t[3];

/*
 * 42 bytes string for 16 consecutive hex formatted bytes.
 */
typedef int8_t h42_t[16 * 3];

/*
 * Hex error codes
 */
typedef enum {
  he_ok,
  he_argc,
  he_state,
  he_number,
  he_read,
  he_size
} he_t;

/*
 * Hex program states
 */
typedef enum {
  hs_ready,
  hs_occupied
} hs_t;

/*
 * Hex byte row
 */
typedef struct {
  h16_t ascii;
  int64_t zero;

  union {
    h3_t hex[16];
    h42_t allhex;
  };

  int64_t zero1;
} hr_t;

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
  // app_t app;
  hex_t hex;
} hexapp_t;

/*******************************************************************************
 *                              Hex functions
 *******************************************************************************/

// /*
//  * Initialize app
//  */
// int h_init(hexapp_t* app);
//
// /*
//  * Deinit app
//  */
// void h_deinit(hexapp_t* app);
//
// /*
//  * Open file
//  */
// int h_open(app_t* app, ha_t* args);
//
// /*
//  * Close file
//  */
// int h_close(app_t* app, ha_t* args);
//
// /*
//  * Move to file offset
//  */
// int h_move(app_t* app, ha_t* args);
//
// /*
//  * View bytes in hexadecimal viewer
//  */
// int h_view(app_t* app, ha_t* args);
//
// /*
//  * Save file offset
//  */
// int h_mark(app_t* app, ha_t* args);
//
// /*
//  * Remove file offset
//  */
// int h_unmark(app_t* app, ha_t* args);
//
// /*
//  * Go to saved offset
//  */
// int h_atmark(app_t* app, ha_t* args);
//
// /*
//  * Quit
//  */
// int h_quit(app_t* app, ha_t* args);
//
// /*
//  * Find a word sequence
//  */
// int h_find(app_t* app, ha_t* args);
//
// /*
//  * Find an hexadecimal byte sequence
//  */
// int h_findx(app_t* app, ha_t* args);
//
// /*
//  * Find images
//  */
// int h_findimg(app_t* app, ha_t* args);
