/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include <assert.h>
#include <stdio.h>

typedef enum {
  ERROR_OK,
  ERROR_FOPEN,
  ERROR_OUT_OF_RANGE,
  ERROR_NO_FILE_LOADED,
  ERROR_INVALID_ARGS,
  ERROR_NOT_ENOUGH_ARGS,
  ERROR_INVALID_NUMBER,
  ERROR_END_OF_FILE,
} error;

/*******************************************************************************
 *                            Error functions
 *******************************************************************************/

int error_fopen(FILE *result, const char *path);
int error_out_of_range(long pos, long length, long low, long high);
int error_invalid_integer(int valid, const char *str);
int error_no_file_loaded(FILE *file);
int error_invalid_arguments(int got_argc, int expected_argc);
int error_not_enough_args(int got_argc, int expected_argc);
int error_end_of_file(FILE *file);

/*******************************************************************************
 *                            Assert Macro
 *******************************************************************************/
#define not_null(x) assert((x) != NULL)
