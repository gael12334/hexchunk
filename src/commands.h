/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "arrayview.h"
#include "error.h"
#include "listview.h"
#include "typedef.h"

#include <ctype.h>

/*******************************************************************************
 *                                  Constants
 *******************************************************************************/
typedef enum : uint8_t {
  CARGINT, // expect an integer
  CARGNUM, // expect a number
  CARGWRD, // expect a whitespace delimited word
  CARGSTR, // expect a string literal surrounded by double-quotes
  CARGHEX  // expect an hexadecimal string
} catype_t;

enum {
  c_default_alloc = 10
};

/*******************************************************************************
 *                               Type definitions
 *******************************************************************************/
typedef int (*cmdfn_t)(void* extra, size_t argc, value_t* argv);

typedef struct {
  cmdfn_t handler;                   // function executed once a match is found
  charview_t pattern;                // the command pattern that must be matched
  arrayviewof_t(catype_t) cargstype; // types of arguments when parser finds a '*' char.
  arrayviewof_t(value_t) carguments; // space used to store argument once parsed
} cmd_t;

typedef struct {
  listviewof_t(cmd_t) commands; // list of commands
} cmdmgr_t;

/*******************************************************************************
 *                                   Macros
 *******************************************************************************/

/*******************************************************************************
 *                                  Functions
 *******************************************************************************/

int c_init(cmdmgr_t* mgr);
int c_deinit(cmdmgr_t* mgr);
int c_register(cmdmgr_t* mgr, charview_t pattern, vaargs_t argstype, cmdfn_t handler);
int c_find(cmdmgr_t* mgr, charview_t pattern, cmd_t* out);
int c_translate(cmd_t* cmd);
int c_execute(cmd_t* cmd);
