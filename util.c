/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 16
#define HEXSTR_SIZE (LINE_LENGTH * 3 + 1)
#define ASCIISTR_SIZE (LINE_LENGTH + 1)

typedef char *rschr;       // reference to signed character
typedef char *pschr;       // pointer to signed characters
typedef const rschr pcchr; // pointer to constant
typedef uint8_t *pubyte;   // pointer to unsigned bytes
typedef int *rsint;        // reference to signed integer
typedef long *rslong;      // reference to signed long

/*******************************************************************************
 *                           Foward declaration
 ********************************************************************************/

int str_is_int(rschr str, rslong out);
void bytebuf_hexview(pubyte buffer, long size, long start_at);
pschr *parse_input(rschr buffer, size_t len, rsint out);
void byte_to_hex(rschr hi, rschr lo, uint8_t byte);

pcchr bytebuf_to_hexstr(pubyte row, size_t len);
pcchr bytebuf_to_asciistr(pubyte row, size_t len);
int bytebuf_is_zeroed(pubyte buffer, long size);

/*******************************************************************************
 *                           Implementation
 ********************************************************************************/

int str_is_int(pschr str, rslong out) {
  int valid = 1;
  int minus = 0;
  int digit = 0;
  int check = 0;

  if (*str == '-')
    str++;

  for (rschr pchr = str; *pchr != '\0' && valid == 1; pchr++) {
    valid &= isdigit(*pchr) ? 1 : 0;
  }

  if (valid) {
    *out = strtol(str, NULL, 10);

    if (minus) {
      *out *= -1;
    }
  }

  return valid;
}

void byte_to_hex(rschr hi, rschr lo, uint8_t byte) {
  static const char lookup[] = "0123456789ABCDEF";
  *hi = lookup[(byte & 0xf0) >> 4];
  *lo = lookup[(byte & 0x0f)];
}

pschr *parse_input(pschr buffer, size_t len, rsint out_argc) {
  assert(out_argc != NULL);
  assert(buffer != NULL);

  rschr input = fgets(buffer, len, stdin);
  if (input == NULL) {
    *out_argc = 0;
    return NULL;
  }

  input[strcspn(input, "\n\r")] = '\0';

  int argc = 0;
  rschr *argv = NULL;
  size_t allocd = 0;

  rschr token = strtok(input, " ");
  while (token != NULL) {
    if (argc == allocd) {
      allocd += (allocd + 1) * 2;
      argv = realloc(argv, sizeof(*argv) * allocd);
      assert(argv != NULL);
    }

    argv[argc] = token;
    token = strtok(NULL, " ");
    argc++;
  }

  *out_argc = argc;
  return argv;
}

void bytebuf_hexview(pubyte buffer, long size, long start_at) {
  long i = 0;
  long len = 0;
  pubyte row;
  rschr hexstr;
  rschr asciistr;

  printf("%-10s ", "address");
  for (long i = 0; i < LINE_LENGTH; i++) {
    printf(".%lX ", i);
  }

  while (i < size) {
    len = (i + LINE_LENGTH < size) ? LINE_LENGTH : size - i;
    row = buffer + i;

    hexstr = bytebuf_to_hexstr(row, len);
    asciistr = bytebuf_to_asciistr(row, len);
    printf("\n%010lx %-48s %s", start_at + i, hexstr, asciistr);

    i += len;
  }

  fputc('\n', stdout);
}

pcchr bytebuf_to_hexstr(pubyte row, size_t len) {
  static char hexstr[HEXSTR_SIZE] = {0};
  rschr hexbyte;
  size_t i;

  for (i = 0; i < len && i < LINE_LENGTH; i++) {
    hexbyte = &hexstr[i * 3];
    byte_to_hex(hexbyte, hexbyte + 1, row[i]);
    hexbyte[2] = ' ';
  }

  while (i < LINE_LENGTH) {
    hexbyte = &hexstr[i * 3];
    memset(hexbyte, 0, 3);
    i++;
  }

  hexstr[HEXSTR_SIZE - 1] = '\0';
  return hexstr;
}

pcchr bytebuf_to_asciistr(pubyte row, size_t len) {
  static char asciistr[ASCIISTR_SIZE];
  size_t i;

  for (i = 0; i < len && i < LINE_LENGTH; i++) {
    if (row[i] < ' ')
      asciistr[i] = '.';
    else if (row[i] > '~')
      asciistr[i] = '.';
    else
      asciistr[i] = row[i];
  }

  while (i < LINE_LENGTH) {
    asciistr[i] = ' ';
    i++;
  }

  asciistr[ASCIISTR_SIZE - 1] = '\0';
  return asciistr;
}

int bytebuf_is_zeroed(pubyte buffer, long size) {
  int zeroed = 1;
  for (long i = 0; i < size && zeroed > 0; i += sizeof(long)) {
    long next_i = i + sizeof(long);
    long filter = (next_i > size) ? (next_i - size) : 0;
    long bytes = *(rslong)&buffer[i];
    bytes >>= filter * sizeof(char);
    zeroed = (bytes == 0);
  }
  return zeroed;
}
