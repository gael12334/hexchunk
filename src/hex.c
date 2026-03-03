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

static void h_byte2hex(int8_t *ascii_ch, h3_t *out) {
  static const int8_t hexchars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  int8_t ch = *ascii_ch;
  int low = ch & 0x0F;
  int high = (ch & 0xF0) >> 4;
  (*out)[0] = hexchars[high];
  (*out)[1] = hexchars[low];
  if (ch < 0x20 || ch >= 0x7F)
    (*ascii_ch) = '.';
  else
    (*ascii_ch) = ch;
}

static void h_empty2hex(int8_t *ascii_ch, h3_t *out) {
  (*out)[0] = ' ';
  (*out)[1] = ' ';
  *ascii_ch = ' ';
}

static void h_readrow(stream_t *s, long *len, hr_t *out, long off, long end) {
  int quad = 0;
  long length = 0;
  out->zero = 0;
  out->zero1 = 0;

  for (size_t i = 0; i < sizeof(out->ascii); i++) {
    long read = 0;
    int err = 0;
    int8_t *ascii_ptr = &out->ascii[i];

    // read only if within desired size
    if (off + i < end) {
      err = s_readbyte(s, ascii_ptr, &read);
    }

    // if read was valid, proceed to conversion
    if (err == 0 && read == 1) {
      h_byte2hex(ascii_ptr, out->hex + i);
      length++;
    } else {
      h_empty2hex(ascii_ptr, out->hex + i);
    }

    // every 4 bytes, display a vertical bar
    quad++;
    out->hex[i][2] = (quad & 4) ? '|' : ' ';
    quad &= 3;
  }

  *len = length;
}

static int h_showhex(stream_t *stream, long size) {
  int err;

  // offset
  long offset;
  err = s_pos(stream, &offset);
  check_he(err, printf("Failed to get stream pos; error code %i\n", err));

  // header
  const char space[] = ".....offset.....";
  const char hxdcm[] = ".0..1..2..3|.4..5..6..7|.8..9..A..B|.C..D..E..F|";
  const char ascii[] = "0123456789ABCDEF";
  printf("%s|%s%s\n", space, hxdcm, ascii);

  // row
  hr_t row = {0};
  long length = 16;
  long start = offset;
  long end = offset + size;

  while (length == 16 && offset < end) {
    h_readrow(stream, &length, &row, offset, end);
    printf("%016lx|%s%s\n", offset, row.allhex, row.ascii);
    offset += length;
  }

  return he_ok;
}

static int h_pos_size(stream_t *stream, long *pos, long *size) {
  int err;

  err = s_pos(stream, pos);
  check_he(err, { printf("Unable to get stream pos; error code %i\n", err); });

  err = s_length(stream, size);
  check_he(err, { printf("Unable to get stream size; error code %i\n", err); });

  return he_ok;
}

static int h_check(app_t *app, ha_t *args, long expctd, hexapp_t **hexapp) {
  assert(app != NULL && args != NULL);
  long argc = args->argc;
  check_args(argc, expctd, { printf("Expected %li arguments.\n", expctd); });

  *hexapp = (hexapp_t *)app;
  check_occupied((*hexapp)->hex.state, { puts("Stream is not in use."); });

  return he_ok;
}

static int h_fndpttrn(hexapp_t *ha, ha_t *args, long pos, long sz, sb_t *pmem) {
  int err;

  // 2nd arg : range
  long range;
  err = a_arg2long(args->argv[2], &range);
  check_he(err, { printf("Failed to parse range; error code %i.\n", err); });

  if (range <= 0 || pos + range > sz) {
    range = sz - pos;
  }

  long match = 0;
  long which = -1;
  stream_t stream;

  err = s_openmem(&stream, pmem, sm_binary_read);
  check_he(err, { printf("Stream of pattern failed; error code: %i\n", err); });

  while (err == se_ok) {
    err = s_seek(&ha->hex.stream, &stream, 1, &which, range);

    if (err == se_ok) {
      err = s_pos(&ha->hex.stream, &pos);
      for (char *c = pmem->data; c != pmem->data + pmem->size; c++) {
        printf("%hhX ", *c);
      }
      printf(" @ %li\n", pos - pmem->size);
      match++;
    }
  }

  s_close(&stream);
  if (err == se_nomatch) {
    if (match > 0) {
      printf("%li matches. \n", match);
      return he_ok;
    }

    else {
      printf("Zero matches. \n");
      return se_nomatch;
    }
  }

  else {
    printf("Error code %i.\n", err);
    return err;
  }
}

/*******************************************************************************
 *                            Hex functions
 *******************************************************************************/

int h_init(hexapp_t *app) {
  assert(app != NULL);

  ac_t commands[] = {
      a_command("open", "open a file", h_open),
      a_command("close", "close loaded file", h_close),
      a_command("move", "move stream position", h_move),
      a_command("view", "view stream bytes", h_view),
      a_command("quit", "close loaded file & quit", h_quit),
      a_command("find", "find a pattern in file", h_find),
      a_command("findx", "find an hex pattern in file", h_findx),
      a_command("help", "The help menu", a_help),
  };

  ap_t ap = {
      .commands = commands,
      .cmdnum = sizeof(commands) / sizeof(commands[0]),
      .name = "hexchunk",
      .version = a_version(1, 0, 0),
  };

  int err = a_init(&app->app, &ap);
  check_he(err, { puts("Failed to load base app."); });
  app->hex.state = hs_ready;
  return he_ok;
}

void h_deinit(hexapp_t *app) {
  assert(app != NULL);

  if (app->hex.state == hs_occupied) {
    p_deinit(&app->hex.path);
    s_close(&app->hex.stream);
  }

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
  check_he(err, {
    printf("Failed to open file; error code %i.\n", err);
    p_deinit(&ha->hex.path);
  });

  ha->hex.state = hs_occupied;
  printf("File '%s' successfully opened.\n", args->argv[1]);
  return he_ok;
}

int h_close(app_t *app, ha_t *args) {
  int err;
  hexapp_t *ha;

  err = h_check(app, args, 1, &ha);
  check_he(err, {});

  char *path;
  p_string(&ha->hex.path, &path);
  printf("Successfully closed %s.\n", path);

  s_close(&ha->hex.stream);
  p_deinit(&ha->hex.path);
  ha->hex.state = hs_ready;

  return he_ok;
}

int h_move(app_t *app, ha_t *args) {
  int err;
  hexapp_t *ha;

  err = h_check(app, args, 2, &ha);
  check_he(err, {});

  long offset;
  err = a_arg2long(args->argv[1], &offset);
  check_he(err, { printf("Failed to parse offset; error code %i.\n", err); });

  err = s_move(&ha->hex.stream, offset);
  check_he(err, { printf("Move to offset failed; error code %i.\n", err); });

  return he_ok;
}

int h_view(app_t *app, ha_t *args) {
  int err;
  hexapp_t *ha;

  err = h_check(app, args, 2, &ha);
  check_he(err, {});

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
  int err;
  hexapp_t *ha;

  err = h_check(app, args, 3, &ha);
  check_he(err, {});

  long position, size;
  err = h_pos_size(&ha->hex.stream, &position, &size);
  check_he(err, {});

  str pattern = args->argv[1];
  sb_t pmem = {.data = pattern, .size = strlen(pattern)};
  return h_fndpttrn(ha, args, position, size, &pmem);
}

int h_findx(app_t *app, ha_t *args) {
  int err;
  hexapp_t *ha;

  err = h_check(app, args, 3, &ha);
  check_he(err, {});

  long position, size;
  err = h_pos_size(&ha->hex.stream, &position, &size);
  check_he(err, {});

  size_t length = strlen(args->argv[1]);
  int8_t *pattern = malloc(sizeof(int8_t) * length);
  assert(pattern != NULL);
  memcpy(pattern, args->argv[1], length);
  size_t pttrnsz = (length + (length & 1)) >> 1;
  size_t dest = length - 1;
  char offset[] = {'0', 'A' - 10, 'a' - 10};
  char limits[] = {'9', 'F' - 10, 'f' - 10};

  for (size_t i = length; i > 0; i--) {
    int8_t ch = pattern[i - 1];
    int8_t type = ((ch & 0x60) >> 5) - 1;
    int8_t odd = (i & 1) != (length & 1);
    int8_t value = ch - offset[type];

    if (value < 0 || value > limits[type]) {
      printf("Invalid digit @ pos %zu (%c)\n", i - 1, pattern[i - 1]);
      return he_number;
    }

    value <<= (odd * 4);
    pattern[dest] *= odd;
    pattern[dest] |= value;
    dest -= odd;
  }

  int8_t *buf = pattern + (length - pttrnsz);
  sb_t pmem = {.data = buf, .size = pttrnsz};
  err = h_fndpttrn(ha, args, position, size, &pmem);
  free(pattern);
  return err;
}

int h_findimg(app_t *app, ha_t *args);

int h_extract(app_t *app, ha_t *args);
