/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "stream.h"

#define check_errno(error, clean)                                              \
  errno = 0;                                                                   \
  if (errno) {                                                                 \
    clean;                                                                     \
    return error;                                                              \
  }

#define check_handle(s, clean)                                                 \
  if (s->handle == NULL) {                                                     \
    clean;                                                                     \
    return se_null;                                                            \
  }

#define check_se(se, clean)                                                    \
  if (se != se_ok) {                                                           \
    clean;                                                                     \
    return se;                                                                 \
  }

#define check_canread(mode, clean)                                             \
  if (!s_canread(mode)) {                                                      \
    clean;                                                                     \
    return se_mode;                                                            \
  }

#define check_null(p, clean)                                                   \
  if (p == NULL) {                                                             \
    clean;                                                                     \
    return se_null;                                                            \
  }

/*******************************************************************************
 *                       Internal utility functions
 *******************************************************************************/

static void *s_alloc(long size) {
  void *out = malloc(size);
  assert(out != NULL);
  memset(out, 0, size);
  return out;
}

static long s_consumable(stream_t *s, long *step, se_t *err) {
  int result = 1;
  result &= (*err = s_poll(s, *step, step)) == se_ok;
  result &= (*step) == 1;
  return result;
}

static long s_match(char s_byte, char m_byte, long matching, long size) {
  int bytes_match = (s_byte == m_byte);
  int size_match = (matching + 1 == size);
  return bytes_match && size_match;
}

static long s_canread(sm_t mode) {
  long readmode = (mode & 0x000000FF) == sm_read;
  long plusmode = (mode & 0x0000FF00) == sm_plus;
  long binpmode = (mode & 0x00FFFF00) == sm_binary_plus;
  return readmode || plusmode || binpmode;
}

static long s_canwrite(sm_t mode) {
  long readmode = (mode & 0x00FFFF) == sm_read;
  long rbinmode = (mode & 0x00FFFF) == sm_binary_read;
  return (!readmode && !rbinmode);
}

static void s_stream_cleanup(stream_t *s) {
  fclose(s->handle);
  memset(s, 0, sizeof(*s));
}

static se_t s_stream(stream_t *out, FILE *handle, st_t type, sm_t mode) {
  out->handle = handle;
  out->mode = mode;
  out->type = type;

  /* https://www.manpagez.com/man/3/fopen/
   *
   * A: already at the end, ftell directly for length
   * W: deletes existing content, length always zero
   * R: starts at the start, fseek end + ftell + rewind
   */

  switch (out->mode & 0x000000FF) {
  case sm_append:
    out->size = ftell(out->handle);
    check_errno(se_stdio, { s_stream_cleanup(out); });
    break;

  case sm_write:
    out->size = 0L;
    break;

  case sm_read:
    fseek(out->handle, 0, SEEK_END);
    check_errno(se_stdio, { s_stream_cleanup(out); });

    out->size = ftell(out->handle);
    check_errno(se_stdio, { s_stream_cleanup(out); });

    rewind(out->handle);
    break;

  default:
    s_stream_cleanup(out);
    return se_mode;
  }

  return se_ok;
}

/*******************************************************************************
 *                            File functions
 *******************************************************************************/

se_t s_openfile(stream_t *out, cstr path, sm_t mode) {
  assert(out != NULL);
  assert(path != NULL);

  size_t len = strlen(path);
  assert(len < PATH_MAX - 1);

  st_t type = st_file;
  FILE *handle = fopen(path, (const char *)&mode);
  check_null(handle, { memset(out, 0, sizeof(*out)); });
  check_errno(se_stdio, { memset(out, 0, sizeof(*out)); });

  return s_stream(out, handle, type, mode);
}

se_t s_openmem(stream_t *out, sb_t *mem, sm_t mode) {
  assert(out != NULL);
  assert(mem != NULL);
  assert(mem->size >= 0);

  st_t type = st_memory;
  FILE *handle = fmemopen(mem->data, mem->size, (cstr)&mode);
  check_null(handle, { memset(out, 0, sizeof(*out)); });
  check_errno(se_stdio, { memset(out, 0, sizeof(*out)); });
  return s_stream(out, handle, type, mode);
}

se_t s_close(stream_t *s) {
  assert(s != NULL);
  check_handle(s, {});

  fclose(s->handle);
  check_errno(se_stdio, {});

  memset(s, 0, sizeof(*s));
  return se_ok;
}

se_t s_flush(stream_t *s) {
  assert(s != NULL);
  check_handle(s, {});

  fflush(s->handle);
  check_errno(se_stdio, {});

  return se_ok;
}

se_t s_length(stream_t *s, long *out) {
  assert(s != NULL);
  assert(out != NULL);

  // absence of break in append and write
  // is intentional.
  switch (s->mode & 0x000000FF) {
  case sm_append:
    fseek(s->handle, 0, SEEK_END);
    check_errno(se_stdio, {});

  case sm_write:
    s->size = ftell(s->handle);
    check_errno(se_stdio, {});

  case sm_read:
    *out = s->size;
    return se_ok;

  default:
    return se_mode;
  }
}

se_t s_read(stream_t *s, sb_t *out, long *read) {
  assert(s != NULL);
  assert(out != NULL);
  assert(out->data != NULL);
  check_handle(s, {});
  check_canread(s->mode, {});

  *read = fread(out->data, 1, out->size, s->handle);
  check_errno(se_stdio, {});
  return se_ok;
}

se_t s_readbyte(stream_t *s, int8_t *out, long *read) {
  sb_t mem = s_primitve(out);
  return s_read(s, &mem, read);
}

se_t s_readshort(stream_t *s, int16_t *out, long *read) {
  sb_t mem = s_primitve(out);
  return s_read(s, &mem, read);
}

se_t s_readinteger(stream_t *s, int32_t *out, long *read) {
  sb_t mem = s_primitve(out);
  return s_read(s, &mem, read);
}

se_t s_readlong(stream_t *s, int64_t *out, long *read) {
  sb_t mem = s_primitve(out);
  return s_read(s, &mem, read);
}

se_t s_write(stream_t *s, sb_t *mem, long *written) {
  assert(s != NULL);
  assert(mem != NULL);
  assert(mem->data != NULL);
  check_handle(s, {});

  *written = fwrite(mem->data, 1, mem->size, s->handle);
  check_errno(se_stdio, {});
  return se_ok;
}

se_t s_poll(stream_t *s, long size, long *out) {
  assert(s != NULL);
  assert(out != NULL);
  check_handle(s, {});

  long append = (s->mode & 0xFF) == sm_append;
  long write = (s->mode & 0xFF) == sm_write;

  // Only reading is limited by the size of the stream.
  // Appending or writing often changes the size of the file.
  if (append || write) {
    *out = size;
    return se_ok;
  }

  long stream_size = s->size;
  long position = ftell(s->handle);
  check_errno(se_stdio, {});

  long new_pos = position + size;
  if (0 <= new_pos && new_pos <= stream_size) {
    *out = size;
    return se_ok;
  } else {
    // returns the excess number of bytes.
    *out = position + size - stream_size;
    return se_size;
  }
}

se_t s_start(stream_t *s) {
  assert(s != NULL);
  check_handle(s, {});

  rewind(s->handle);
  return se_ok;
}

se_t s_end(stream_t *s) {
  assert(s != NULL);
  check_handle(s, {});

  fseek(s->handle, 0, SEEK_END);
  check_errno(se_stdio, {});
  return se_ok;
}

se_t s_pos(stream_t *s, long *out) {
  assert(s != NULL);
  check_handle(s, {});

  *out = ftell(s->handle);
  check_errno(se_stdio, {});
  return se_ok;
}

se_t s_move(stream_t *s, long where) {
  assert(s != NULL);
  check_handle(s, {});
  check_canread(s->mode, {});

  fseek(s->handle, where, SEEK_SET);
  check_errno(se_stdio, {});
  return se_ok;
}

se_t s_push(stream_t *s, long size) {
  assert(s != NULL);
  check_handle(s, {});
  check_canread(s->mode, {});

  se_t se = s_poll(s, size, &size);
  check_se(se, {});

  long where = ftell(s->handle) + size;
  check_errno(se_stdio, {});

  fseek(s->handle, where, SEEK_SET);
  check_errno(se_stdio, {});

  return se_ok;
}

se_t s_pop(stream_t *s, long size) {
  assert(s != NULL);
  check_handle(s, {});
  check_canread(s->mode, {});

  size *= -1;
  se_t se = s_poll(s, size, &size);
  check_se(se, {});

  long where = ftell(s->handle) + size;
  check_errno(se_stdio, {});

  fseek(s->handle, where, SEEK_SET);
  check_errno(se_stdio, {});

  return se_ok;
}

se_t s_seek(stream_t *s, stream_t *needlelist, size_t num, long *which) {
  assert(s != NULL);
  assert(needlelist != NULL);
  assert(which != NULL);
  check_canread(s->mode, {});

  se_t err;
  long hint = 0;
  long read;
  long step = 1;
  long *matching = s_alloc(sizeof(long) * num);
  long *count;
  int8_t s_byte;
  int8_t m_byte;

  while (s_consumable(s, &step, &err)) {
    err = s_readbyte(s, &s_byte, &read);
    check_se(err, {
      hint = 1;
      goto exception;
    });

    for (stream_t *needle = needlelist; needle != needlelist + num; needle++) {
      int consumable = !s_consumable(needle, &step, &err);
      check_se(err, {
        hint = 2;
        goto exception;
      });
      if (consumable) {
        s_start(needle);
        continue;
      }

      long needle_index = needle - needlelist;
      count = &matching[needle_index];
      err = s_readbyte(needle, &m_byte, &read);
      check_se(err, {
        hint = 3;
        goto exception;
      });

      if (s_match(s_byte, m_byte, *count, needle->size)) {
        *which = needle_index;
        free(matching);
        return se_ok;
      }

      else if (s_byte == m_byte) {
        (*count)++;
      }

      else {
        (*count) = 0;
        s_start(needle);
      }
    }
  }

  check_se(err, {
    hint = 4;
    goto exception;
  });
  free(matching);
  return se_nomatch;

exception:
  free(matching);
  return err;
}

se_t s_consumed(stream_t *s) {
  assert(s != NULL);

  int status = feof(s->handle);
  check_errno(se_stdio, {});

  return status == 0 ? se_ok : se_consumed;
}
