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

#define check_read(read, exp, clean)                                           \
  if (read != exp) {                                                           \
    clean;                                                                     \
    return he_read;                                                            \
  }

/*******************************************************************************
 *                            Internal functions
 *******************************************************************************/

static void h_readrow(stream_t *s, long *len, hr_t *out, long off, long size) {
  static const int8_t hexchars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  int8_t *pch;
  long read = 0;
  int err = 0;
  int quad = 0;

  *len = 0;
  out->zero = 0;
  out->zero1 = 0;
  for (size_t i = 0; i < sizeof(out->ascii); i++) {
    pch = &out->ascii[i];
    err = s_readbyte(s, pch, &read);

    if (err == 0 && read == 1 && off + i < size) {
      int low = *pch & 0x0F;
      int high = (*pch & 0xF0) >> 4;
      out->hex[i][0] = hexchars[high];
      out->hex[i][1] = hexchars[low];
      (*pch) = ((*pch) < 0x20 || (*pch) > 0x7E) ? '.' : *pch;
      (*len)++;
    }

    else {
      out->hex[i][0] = ' ';
      out->hex[i][1] = ' ';
      *pch = ' ';
    }

    quad++;
    out->hex[i][2] = (quad == 4) ? '|' : ' ';
    quad *= (quad != 4);
  }
  return;
}

static int h_showhex(stream_t *stream, long size) {
  int err;

  // offset
  long offset;
  err = s_pos(stream, &offset);
  check_he(err, { printf("Failed to get stream pos; error code %i\n", err); });

  // header
  const char space[] = ".....offset.....";
  const char hxdcm[] = ".0..1..2..3|.4..5..6..7|.8..9..A..B|.C..D..E..F|";
  const char ascii[] = "0123456789ABCDEF";
  printf("%s|%s%s\n", space, hxdcm, ascii);

  // row
  hr_t row = {0};
  long length = 16;
  long start = offset;

  while (length == 16 && offset - start < size) {
    h_readrow(stream, &length, &row, offset, size);
    printf("%016lx|%s%s\n", offset, row.allhex, row.ascii);
    offset += length;

    // Length can be less than 16 if `size` is reached
    //
    // if (length != 16) {
    //   printf("Warning: read %li bytes\n", length);
    // }
  }

  return he_ok;
}

/*******************************************************************************
 *                            Hex functions
 *******************************************************************************/

int h_init(hexapp_t *app) {
  assert(app != NULL);

  ac_t commands[] = {a_command("open", "open a file", h_open),
                     a_command("close", "close loaded file", h_close),
                     a_command("move", "move stream position", h_move),
                     a_command("view", "view stream bytes", h_view),
                     a_command("quit", "close loaded file & quit", h_quit),
                     a_command("help", "The help menu", a_help)};

  ap_t ap = {
      .commands = commands,
      .cmdnum = sizeof(commands) / sizeof(commands[0]),
      .name = "hexchunk",
      .version = a_version(1, 0, 0),
  };

  a_init(&app->app, &ap);
  app->hex.state = hs_ready;
  return he_ok;
}

void h_deinit(hexapp_t *app) {
  str args[1];
  aa_t aa = a_args(1, args);
  s_close(&app->hex.stream);
  a_deinit(&app->app);
  memset(app, 0, sizeof(*app));
}

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
  p_string(&ha->hex.path, &path);

  ha->hex.state = hs_ready;
  printf("Successfully closed %s.\n", path);
  memset(&ha->hex, 0, sizeof(ha->hex));
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
    return he_size;
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

int h_quit(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);
  app->closed = ae_closed;
  return he_ok;
}

int h_find(app_t *app, ha_t *args) {
  assert(app != NULL);
  assert(args != NULL);

  hexapp_t *ha = (hexapp_t *)app;
  int err;

  check_args(args->argc, 3, { puts("Expected 3 arguments."); });
  check_occupied(ha->hex.state, { puts("Stream is unused."); });

  long position;
  err = s_pos(&ha->hex.stream, &position);
  check_he(err, { printf("Unable to get stream pos; error code %i\n", err); });

  long size;
  err = s_pos(&ha->hex.stream, &size);
  check_he(err, { printf("Unable to get stream size; error code %i\n", err); });

  // 1st arg : pattern
  str pattern = args->argv[1];

  // 2nd arg : range
  long range;
  err = a_arg2long(args->argv[2], &range);
  check_he(err, { printf("Failed to parse range; error code %i.\n", err); });

  if (position + range > size || range <= 0) {
    range = size - position;
  }

  long which = -1;
  sb_t memory = {.data = pattern, .size = strlen(pattern)};
  stream_t stream = {0};
  err = s_openmem(&stream, &memory, sm_binary_read);
  check_he(err, { printf("Stream of pattern failed; error code: %i\n", err); });

  err = s_seek(&ha->hex.stream, &stream, 1, &which);
  s_close(&stream);
  check_he(err, { printf("Seek failed; error code: %i\n", err); });

  err = s_pos(&ha->hex.stream, &position);
  check_he(err, { printf("Unable to get stream pos; error code %i\n", err); });

  printf("Found pattern at offset %li.\n", position - memory.size);
  return he_ok;
}

int h_findimg(app_t *app, ha_t *args);
