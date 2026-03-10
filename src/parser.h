/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#pragma once
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "typedef.h"

/*******************************************************************************
 *                             Parser functions
 *******************************************************************************/

int p_parse_integer(char* text, char** end, size_t n, int64_t* i64);

int p_parse_hexint(char* text, char** end, size_t n, int64_t* i64);

int p_parse_float(char* text, char** end, size_t n, double* f64);

int p_parse_keyword(char* text, char** begin, char** end, size_t n);

int p_parse_string(char* text, char** begin, char** end, size_t n);

int p_parse_hexstr(char* text, char** begin, char** end, size_t n, size_t* size);

int p_parse_hexadecimal(char* begin, char* end, int8_t* bytes, size_t n_bytes);
