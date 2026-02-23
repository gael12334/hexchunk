/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "error.h"

#include <stdint.h>
#include <stdio.h>

int error_fopen(FILE *result, const char *path) {
  if (result)
    return ERROR_OK;

  printf("error: fopen: failed to open '%s'.\n", path);
  return ERROR_FOPEN;
}

int error_out_of_range(long pos, long length, long low, long high) {
  if (low <= pos + length && pos + length <= high)
    return ERROR_OK;

  printf("error: 'pos + length' (%li + %li = %li) is out of range (%li:%li).\n",
         pos, length, pos + length, low, high);
  return ERROR_OUT_OF_RANGE;
}

int error_invalid_integer(int valid, const char *str) {
  if (valid)
    return ERROR_OK;

  printf("error: string (%s) could not be converted to an integer.\n", str);
  return ERROR_INVALID_NUMBER;
}

int error_no_file_loaded(FILE *file) {
  if (file)
    return ERROR_OK;

  puts("error: no file loaded.");
  return ERROR_NO_FILE_LOADED;
}

int error_invalid_arguments(int got_argc, int expected_argc) {
  if (got_argc == expected_argc)
    return ERROR_OK;

  printf("error: expected %i args, got %i\n", expected_argc, got_argc);
  return ERROR_INVALID_ARGS;
}

int error_not_enough_args(int got_argc, int expected_argc) {
  if (got_argc >= expected_argc)
    return ERROR_OK;

  printf("error: expected at least %i args, got %i\n", expected_argc, got_argc);
  return ERROR_NOT_ENOUGH_ARGS;
}

int error_end_of_file(FILE *file) {
  if (feof(file) == 0) {
    return ERROR_OK;
  }

  puts("error: end of file reached.");
  return ERROR_END_OF_FILE;
}
