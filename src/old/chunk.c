/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <assert.h>
#include <ctype.h>
#include <linux/limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define CHUNK_SIZE 4096

typedef struct {
  char data[CHUNK_SIZE];
  long size;
} Chunk;

/*******************************************************************************
 *                            Chunk functions
 *******************************************************************************/

void chunk_reset(Chunk *chunk) {
  not_null(chunk);
  memset(chunk, 0, sizeof(*chunk));
}

int chunk_read(FILE *stream, Chunk *chunk) {
  not_null(stream);
  not_null(chunk);

  int opt;
  if ((opt = error_end_of_file(stream)))
    return opt;

  chunk_reset(chunk);
  uint64_t read = fread(chunk->data, sizeof(uint8_t), CHUNK_SIZE, stream);
  chunk->size = (long)read;
  return ERROR_OK;
}

int chunk_read_many(FILE *stream, Chunk *chunks, size_t count) {
  not_null(stream);
  not_null(chunks);

  int opt;

  for (size_t i = 0; i < count; i++) {
    Chunk *c = &chunks[i];

    if ((opt = chunk_read(stream, c)))
      return opt;

    if (c->size < CHUNK_SIZE)
      break;
  }

  return ERROR_OK;
}

int chunk_has_data(Chunk *chunk) {
  not_null(chunk);

  uint64_t *value = NULL;
  uint64_t mask = sizeof(uint64_t) * 2 - 1;
  size_t step = sizeof(uint64_t);
  size_t i = 0;
  int zeroed = 1;
  int exceeds = 0;

  while (step && zeroed) {
    value = (uint64_t *)&chunk->data[i];
    zeroed = (*value & mask) == 0;

    do {
      exceeds = (i + step >= chunk->size);
      step >>= exceeds;
      mask >>= exceeds;
    } while (exceeds);
  }

  return !zeroed;
}

int chunk_memcspn(Chunk *chunk, const uint8_t *reject, size_t num) {
  enum lookup : uint8_t { REJECTED, ACCEPTED };

  enum lookup table[sizeof(uint8_t)];
  memset(table, ACCEPTED, sizeof(table));

  for (size_t i = 0; i < num; i++)
    table[reject[i]] = REJECTED;

  for (size_t i = 0; i < chunk->size; i += sizeof(uint64_t)) {
    uint64_t packed = *(uint64_t *)&chunk->data[i];
    uint64_t next_i = i + sizeof(uint64_t);
    uint64_t lshift = (next_i - chunk->size) * sizeof(uint8_t);
    packed >>= (next_i > chunk->size) * lshift;
  }

  return ERROR_OK;
}

int chunk_find_text(Chunk *chunk, long *index, long *length) {
  not_null(chunk);
  not_null(index);
  not_null(length);

  *index = -1;
  *length = -1;
}

int chunk_find_wide_text(Chunk *chunk, long *index, long *length) {}
