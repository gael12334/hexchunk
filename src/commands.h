/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include "listview.h"
#include "typedef.h"

/*
 * int fait_qqchose_i(int* quit, int argc, valeur_t* argv) { }
 *
 * int fait_qqchose_x(int* quit, int argc, valeur_t* argv) { }
 *
 * #define c_match(str) "%m\" #str "\" "
 * #define c_integer "%i "
 * #define c_number "%f "
 * #define c_string "%s "
 * #define c_hex "%x "
 *
 * int main() {
 *   char format[] = c_match("find") c_string c_integer;
 *   register(format, sizeof(format), fait_qqchose_i)
 *
 *   cleanup
 * }
 */

/*******************************************************************************
 *                            Artificial type definitions
 *******************************************************************************/

// Command handler
typedef int (*cmdfunc_t)(int* quit, uint64_t argc, struct value* argv);

// Command format object
struct cmdformat {
  const char* opt;
  const char* end;

  enum {
    c_m,
    c_i,
    c_f,
    c_s,
    c_x
  } type;
};

// Command object
struct cmdobj {
  cmdfunc_t handler;
  size_t length;
  struct cmdformat* format;
  uint64_t reserved;
  listviewof_t(struct value) args;
};

// Command manager
struct cmdmgr {
  listviewof_t(struct cmdobj) commands; // collection of commands
};

/*******************************************************************************
 *                            Public macros
 *******************************************************************************/

#define c_match(str)                                                                     \
  (struct cmdformat) {                                                                   \
    .opt = str, .end = strlen(str) + 1, .type = c_m                                      \
  }
#define c_integer                                                                        \
  (struct cmdformat) {                                                                   \
    .opt = NULL, .end = NULL, .type = c_i                                                \
  }
#define c_number                                                                         \
  (struct cmdformat) {                                                                   \
    .opt = NULL, .end = NULL, .type = c_f                                                \
  }
#define c_string                                                                         \
  (struct cmdformat) {                                                                   \
    .opt = NULL, .end = NULL, .type = c_s                                                \
  }
#define c_hex                                                                            \
  (struct cmdformat) {                                                                   \
    .opt = NULL, .end = NULL, .type = c_x                                                \
  }

/*******************************************************************************
 *                          Commands functions
 *******************************************************************************/

int c_init(struct cmdmgr* cr);
int c_deinit(struct cmdmgr* cr);
int c_subscribe(struct cmdmgr* cr, struct cmdobj* co);
int c_notify(struct cmdmgr* cr, char* command, size_t size);
