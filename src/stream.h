/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once

#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedef.h"

/*******************************************************************************
 *                            Stream object definitions
 *******************************************************************************/

/*
 * Stream error code
 */
typedef enum {
  se_ok,
  se_stdio,
  se_stdlib,
  se_null,
  se_mem,
  se_size,
  se_pos,
  se_consumed,
  se_nomatch,
  se_mode,
  se_num
} se_t;

/*
 * Stream mode
 */
typedef enum : uint64_t {
  sm_read = u32_wrap(0x00, 0x00, 0x00, 'r'),
  sm_write = u32_wrap(0x00, 0x00, 0x00, 'w'),
  sm_append = u32_wrap(0x00, 0x00, 0x00, 'a'),
  sm_readplus = u32_wrap(0x00, 0x00, '+', 'r'),
  sm_writeplus = u32_wrap(0x00, 0x00, '+', 'w'),
  sm_appendplus = u32_wrap(0x00, 0x00, '+', 'a'),
  sm_binary_read = u32_wrap(0x00, 0x00, 'b', 'r'),
  sm_binary_write = u32_wrap(0x00, 0x00, 'b', 'w'),
  sm_binary_append = u32_wrap(0x00, 0x00, 'b', 'a'),
  sm_binary_readplus = u32_wrap(0x00, '+', 'b', 'r'),
  sm_binary_writeplus = u32_wrap(0x00, '+', 'b', 'w'),
  sm_binary_appendplus = u32_wrap(0x00, '+', 'b', 'a'),
  sm_plus = u32_wrap(0x00, 0x00, '+', 0x00),
  sm_binary = u32_wrap(0x00, 0x00, 'b', 0x00),
  sm_binary_plus = u32_wrap(0x00, '+', 'b', 0x00),
} sm_t;

#define sm_mode(mode) (mode & u32_wrap(0x00, 0x00, 0x00, 0xFF))
#define sm_options(mode) (mode & u32_wrap(0x00, 0xFF, 0xFF, 0x00))

/*
 * Stream type
 */
typedef enum : uint64_t {
  st_file,
  st_memory,
} st_t;

/*
 * Stream memory block
 */
typedef struct {
  void *data;
  long size;
} sb_t;

#define s_primitve(v)                                                          \
  (sb_t) { .data = v, .size = sizeof(*v) }
#define s_array(v)                                                             \
  (sb_t) { .data = v, .size = sizeof(v) }

/*
 * Stream
 */
typedef struct {
  FILE *handle;
  int64_t size;
  sm_t mode;
  st_t type;
} stream_t;

/*******************************************************************************
 *                            Stream functions
 *******************************************************************************/

/*
 * Open a file stream
 */
se_t s_openfile(stream_t *out, cstr path, sm_t mode);

/*
 * Open a memory stream
 */
se_t s_openmem(stream_t *out, sb_t *mem, sm_t mode);

/*
 * Close stream
 */
se_t s_close(stream_t *s);

/*
 * Flush the stream contents
 */
se_t s_flush(stream_t *s);

/*
 * Get the stream length
 */
se_t s_length(stream_t *s, long *out);

/*
 * Read data from the stream
 */
se_t s_read(stream_t *s, sb_t *out, long *read);

/*
 * Read data from the stream
 */
se_t s_readbyte(stream_t *s, int8_t *out, long *read);

/*
 * Read data from the stream
 */
se_t s_readshort(stream_t *s, int16_t *out, long *read);

/*
 * Read a 32 bit integer from the stream
 */
se_t s_readinteger(stream_t *s, int32_t *out, long *read);

/*
 * Read a 64 bit integer from the stream
 */
se_t s_readlong(stream_t *s, int64_t *out, long *read);

/*
 * Write data to the stream
 */
se_t s_write(stream_t *s, sb_t *mem, long *written);

/*
 * Check if stream has at least `size` byte left to consume
 */
se_t s_poll(stream_t *s, long size, long *out);

/*
 * Reset the stream at the start
 */
se_t s_start(stream_t *s);

/*
 * Reset the stream at the end
 */
se_t s_end(stream_t *s);

/*
 * Get where the stream is
 */
se_t s_pos(stream_t *s, long *out);

/*
 * Set where the stream will be
 */
se_t s_move(stream_t *s, long where);

/*
 * Consumes `size` bytes.
 */
se_t s_push(stream_t *s, long size);

/*
 * Throw-up `size` bytes.
 */
se_t s_pop(stream_t *s, long size);

/*
 * Attempts to find one pattern from `mem` in the stream
 */
se_t s_seek(stream_t *s, stream_t *mem, size_t num, long *which);

/*
 * Check if stream reached end-of-file
 */
se_t s_consumed(stream_t *st);
