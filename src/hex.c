/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "hex.h"

#define check_ready(state, clean)                                              \
  if (state != hs_ready) {                                                     \
    clean;                                                                     \
    return he_state;                                                           \
  }

#define check_occupied(state, clean)                                           \
  if (state != hs_occupied) {                                                  \
    clean;                                                                     \
    return he_state;                                                           \
  }

#define check_args(argc, exp, clean)                                           \
  if (argc != exp) {                                                           \
    clean;                                                                     \
    return he_argc;                                                            \
  }

#define check_he(he, clean)                                                    \
  if (he != he_ok) {                                                           \
    clean;                                                                     \
    return he;                                                                 \
  }

/*******************************************************************************
 *                            Internal functions
 *******************************************************************************/

#define h_linewd 16
#define h_addrsz 16
#define h_hexbsz (3 * h_linewd)
#define h_ascisz (1 * h_linewd)
#define h_spacsz 1
#define h_linesz h_addrsz + h_spacsz + h_hexbsz + h_spacsz + h_ascisz + 2

static int h_showhex(stream_t *stream, long size) {
  static char line[h_linesz];
  static cstr xfmt[2] = {"%02hhX ", "%-3c"};
  int a = 0;
  int err;

  // offset
  long offset;
  err = s_pos(stream, &offset);
  check_he(err, { printf("Failed to get stream pos; error code %i\n", err); });

  // header
  a = sprintf(line, "%-16s", "offset");
  a += sprintf(line + a, " ");
  for (size_t i = 0; i < h_linewd; i++) {
    a += sprintf(line + a, "%02zx-", i);
  }
  a += sprintf(line + a, " ");
  a += sprintf(line + a, "%-16s", "ascii");
  printf("%s\n", line);

  // size
  long stream_size;
  err = s_length(stream, &stream_size);
  check_he(err, { printf("Failed to get stream size; error code %i\n", err); });

  // row
  for (long r = 0; s_consumed(stream) != se_consumed; r += h_linewd) {
    // TODO
    // render bytes into hex
    printf("%s\n", line);
  }

  return he_ok;
}

/*******************************************************************************
 *                            Hex functions
 *******************************************************************************/

int h_open(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);
  hexapp_t *ha = (hexapp_t *)app;
  int err;

  check_args(args->argc, 2, { puts("Expected 2 arguments."); });
  check_ready(ha->hex.state, { puts("Stream is already in use."); });

  ps_t ps = p_decayed(args->argv[1]);
  err = p_init(&ha->hex.path, &ps);
  check_he(err, { printf("Path is invalid; error code %i.\n", err); });

  err = s_openfile(&ha->hex.stream, args->argv[1], sm_binary_read);
  check_he(err, { printf("Failed to open file; error code %i.\n", err); });

  ha->hex.state = hs_occupied;
  printf("File '%s' successfully opened.\n", args->argv[1]);
  return he_ok;
}

int h_close(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);
  hexapp_t *ha = (hexapp_t *)app;
  int err;

  check_args(args->argc, 1, { puts("Expected 1 arguments."); });
  check_occupied(ha->hex.state, { puts("Stream is unused."); });

  err = s_close(&ha->hex.stream);
  check_he(err, { printf("Failed to close file; error code %i.\n", err); });

  char *path;
  err = p_string(&ha->hex.path, &path);
  check_he(err, { printf("Path data unreachable; error code %i.\n", err); });

  ha->hex.state = hs_ready;
  printf("File '%s' successfully closed.\n", path);
  return he_ok;
}

int h_move(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);
  hexapp_t *ha = (hexapp_t *)app;
  int err;

  check_args(args->argc, 2, { puts("Expected 2 arguments."); });
  check_occupied(ha->hex.state, { puts("Stream is unused."); });

  long offset;
  err = a_arg2long(args->argv[1], &offset);
  check_he(err, { printf("Failed to parse offset; error code %i.\n", err); });

  err = s_move(&ha->hex.stream, offset);
  check_he(err, { printf("Move to offset failed; error code %i.\n", err); });

  return he_ok;
}

int h_view(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);
  hexapp_t *ha = (hexapp_t *)app;
  int err;

  check_args(args->argc, 2, { puts("Expected 2 arguments."); });
  check_occupied(ha->hex.state, { puts("Stream is unused."); });

  long size;
  err = a_arg2long(args->argv[1], &size);
  check_he(err, { printf("Failed to parse offset; error code %i.\n", err); });

  if (size < 0) {
    puts("Size must be positive.");
    return ae_size;
  }

  if (size > 4096) {
    puts("Warning: viewing is limited to 4096 bytes at a time.");
    size = 4096;
  }

  return h_showhex(&ha->hex.stream, size);
}

int h_mark(app_t *app, ha_t *args);

int h_unmark(app_t *app, ha_t *args);

int h_atmark(app_t *app, ha_t *args);

int h_quit(app_t *app, ha_t *args);

int h_find(app_t *app, ha_t *args);

int h_findimg(app_t *app, ha_t *args);
