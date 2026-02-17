/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <stdint.h>
#include <stdio.h>

#define OK 0
#define ERR_FOPEN 1
#define ERR_FILE_NULL 2
#define ERR_RANGE 3
#define ERR_ARGC 4
#define ERR_CMD 5
#define ERR_NUMBER 6
#define INPUT_BUFFER 512
#define LINE_LENGTH 16
#define BYTE_LENGTH 3
#define OFFSET_LENGTH 6
#define MAX_POINTERS 10

int error_fopen(FILE *result, const char *path) {
  if (result) {
    return OK;
  }

  printf("error: fopen: failed to open '%s'.\n", path);
  return ERR_FOPEN;
}

int error_out_of_range(long pos, long length, long low, long high) {
  if (low <= pos + length && pos + length <= high) {
    return OK;
  }

  printf("error: 'pos + length' (%li + %li = %li) is out of range (%li:%li).\n",
         pos, length, pos + length, low, high);
  return ERR_RANGE;
}

int error_invalid_integer(int valid, const char *str) {
  if (valid) {
    return OK;
  }

  printf("error: string (%s) could not be converted to an integer.\n", str);
  return ERR_NUMBER;
}

int error_no_file_loaded(FILE *file) {
  if (file) {
    return OK;
  }

  puts("error: no file loaded.");
  return ERR_FILE_NULL;
}

int error_invalid_arguments(int got_argc, int expected_argc) {
  if (got_argc == expected_argc) {
    return OK;
  }

  printf("error: expected %i args, got %i\n", expected_argc, got_argc);
  return ERR_ARGC;
}

int error_not_enough_args(int got_argc, int expected_argc) {
  if (got_argc >= expected_argc) {
    return OK;
  }

  printf("error: expected at least %i args, got %i\n", expected_argc, got_argc);
  return ERR_ARGC;
}
