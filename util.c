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

int str_is_int(char *str, long *out) {
  int valid = 1;

  for (char *pchr = str; *pchr != '\0' && valid == 1; pchr++) {
    valid &= isdigit(*pchr) ? 1 : 0;
  }

  if (valid) {
    *out = strtol(str, NULL, 10);
  }

  return valid;
}

void byte_to_hex(char *hi, char *lo, uint8_t byte) {
  static const char lookup[] = "0123456789ABCDEF";
  *hi = lookup[(byte & 0xf0) >> 4];
  *lo = lookup[(byte & 0x0f)];
}

const char *bytebuf_to_hexstr(uint8_t *row, size_t len) {
  static char hexstr[HEXSTR_SIZE] = {0};
  char *hexbyte;
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

const char *bytebuf_to_asciistr(uint8_t *row, size_t len) {
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

char *clistr_to_cstr(char *clistr) {
  //   TODO
  //
  //   char ascii_buf[4] = {0};
  //   long character;
  //   char *corrected = clistr;
  //   char *cursor = clistr;
  //
  //   while (*cursor != '\0') {
  //
  //     char* it1 = it;
  //     while (*it == '\\' && (end - it) < 4) {
  //       if (*end == '\0' || !isdigit(*end))
  //         return NULL;
  //       end++;
  //     }
  //
  //     char ch = 0;
  //     char *it2 = end;
  //     int tenth = 1;
  //
  //     while (it2 != it) {
  //       ch += ((*it2) - '0') * tenth;
  //       tenth *= 10;
  //       it2--;
  //     }
  //
  //     *it = ch;
  //     it++;
  //     end++;
  //   }

  return clistr;
}

void show_buffer_hex(uint8_t *buffer, long size, long start_at) {
  long i = 0;
  long len = 0;
  uint8_t *row;
  const char *hexstr;
  const char *asciistr;

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

char **parse_input(char *buffer, size_t len, int *out) {
  assert(out != NULL);
  assert(buffer != NULL);

  char *input = fgets(buffer, len, stdin);
  if (input == NULL) {
    *out = 0;
    return NULL;
  }

  input[strcspn(input, "\n\r")] = '\0';

  int argc = 0;
  char **argv = NULL;
  size_t allocd = 0;

  char *token = strtok(input, " ");
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

  *out = argc;
  return argv;
}
