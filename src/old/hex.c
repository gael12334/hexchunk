/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define CHUNK 8192
#define OK 0
#define ERR 1

typedef uint8_t chunk8K[CHUNK];

typedef struct {
  chunk8K data;
  long off_here;
  long off_prev;
  long off_next;
  long is_last;
  FILE *filedesc;
  long filesize;
} ChunkIter;

struct {
  long offset;
  long used;
  uint8_t data[CHUNK];
} chunk = {0};

struct {
  const char *path;
  FILE *file;
  long size;
} program = {0};

/*******************************************************************************
 *                            Util functions
 *******************************************************************************/

extern int str_is_int(char *str, long *out);
extern void byte_to_hex(char *hi, char *lo, uint8_t byte);
extern const char *bytebuf_to_hexstr(uint8_t *row, size_t len);
extern const char *bytebuf_to_asciistr(uint8_t *row, size_t len);
extern void bytebuf_hexview(uint8_t *buffer, long size, long start_at);
extern char **parse_input(char *buffer, size_t len, int *out);
extern int bytebuf_is_zeroed(uint8_t *buffer, long size);

/*******************************************************************************
 *                            File functions
 *******************************************************************************/

/*******************************************************************************
                             Chunk functions
********************************************************************************/

int read_chunk(ChunkIter *iterator) {
  int opt;

  long pos = iterator->off_here;
  long size = iterator->off_next - iterator->off_here;
  long filesize = iterator->filesize;

  if ((opt = error_out_of_range(pos, size, 0, filesize - 1)))
    return opt;

  fread(iterator->data, sizeof(uint8_t), size, iterator->filedesc);
  return OK;
}

int next_chunk(ChunkIter *iterator) {
  int opt;
  if ((opt = error_no_file_loaded(iterator->filedesc)))
    return opt;

  if ((opt = read_chunk(iterator)))
    return opt;

  iterator->off_prev = iterator->off_here;
  iterator->off_here = iterator->off_next;
  iterator->off_next += CHUNK;

  if (iterator->off_next > iterator->filesize) {
    iterator->off_next = iterator->filesize - iterator->off_next - 1;
    iterator->is_last = 1;
  }

  return OK;
}

// TODO: implement
int scan_chunk(ChunkIter *iterator, FILE *output) {
  assert(iterator != NULL);

  return OK;
}

// TODO: implement
int scan_many_chunk(const char *entered_path, long chunks) { return OK; }

// TODO: implement
int skip_chunks(long chunks) { return OK; }
