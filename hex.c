/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <assert.h>
#include <ctype.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK 1024
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
                             Util functions
********************************************************************************/

extern int str_is_int(char *str, long *out);
extern void byte_to_hex(char *hi, char *lo, uint8_t byte);
extern const char *bytebuf_to_hexstr(uint8_t *row, size_t len);
extern const char *bytebuf_to_asciistr(uint8_t *row, size_t len);
extern void show_buffer_hex(uint8_t *buffer, long size, long start_at);
extern char **parse_input(char *buffer, size_t len, int *out);
extern char *clistr_to_cstr(char *clistr, size_t len);

/*******************************************************************************
                              Error functions
 ********************************************************************************/

extern int error_fopen(FILE *result, const char *path);
extern int error_out_of_range(long pos, long length, long low, long high);
extern int error_invalid_integer(int valid, const char *str);
extern int error_no_file_loaded(FILE *file);
extern int error_invalid_arguments(int got_argc, int expected_argc);

/*******************************************************************************
                             Chunk functions
********************************************************************************/

int update_chunk(long size) {
  int opt;
  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  long pos = ftell(program.file);
  if ((opt = error_out_of_range(pos, size, 0, program.size - 1)))
    return opt;

  if (size < 0) {
    fseek(program.file, size, SEEK_CUR);
    fread(chunk.data, -size, 1, program.file);
    fseek(program.file, size, SEEK_CUR);
    chunk.used = -size;
    chunk.offset = ftell(program.file);
    return OK;
  }

  fread(chunk.data, size, 1, program.file);
  chunk.used = size;
  chunk.offset = pos;
  return OK;
}

/*******************************************************************************
                             File functions
********************************************************************************/

int close_file(void) {
  int opt;

  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  fclose(program.file);
  program.file = NULL;
  program.size = 0;
  return OK;
}

int open_file(const char *path) {
  static char real_path[PATH_MAX];
  int opt;

  if (program.file != NULL) {
    if ((opt = close_file()))
      return opt;
  }

  realpath(path, real_path);
  program.file = fopen(real_path, "rb");
  if ((opt = error_fopen(program.file, real_path)))
    return opt;

  fseek(program.file, 0, SEEK_END);
  program.size = ftell(program.file);
  program.path = path;

  rewind(program.file);
  return OK;
}

int file_at(long off) {
  int opt;

  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  fseek(program.file, 0, SEEK_SET);
  fseek(program.file, off, SEEK_SET);
  return OK;
}

/*******************************************************************************
                             Command functions
********************************************************************************/

int cmd_open(int argc, char **argv) {
  static const char CMD[] = "open";
  int opt;

  if ((opt = error_invalid_arguments(argc, 2)))
    return opt;

  if ((opt = open_file(argv[1])))
    return opt;

  printf("%s: file %s loaded\n", CMD, argv[1]);
  return OK;
}

/*******************************************************************************
                             CLOSE
*/

int cmd_close(int argc, char **argv) {
  static const char CMD[] = "close";
  int opt;

  if ((opt = close_file()))
    return opt;

  printf("%s: file %s unloaded\n", CMD, program.path);
  return OK;
}

/*******************************************************************************
                             SET
*/

int cmd_set(int argc, char **argv) {
  long off, size;
  int opt, valid;

  if ((opt = error_invalid_arguments(argc, 2)))
    return opt;

  valid = str_is_int(argv[1], &off);
  if ((opt = error_invalid_integer(valid, argv[1])))
    return opt;

  if ((opt = file_at(off)))
    return opt;

  return OK;
}

/*******************************************************************************
                             SKIP
*/

int cmd_skip(int argc, char **argv) {
  long size;
  int opt, valid;

  if ((opt = error_invalid_arguments(argc, 2)))
    return opt;

  valid = str_is_int(argv[1], &size);
  if ((opt = error_invalid_integer(valid, argv[1])))
    return opt;

  if (strcmp(argv[0], "next") == 0) {
    if ((opt = error_out_of_range(0, size, -CHUNK, CHUNK)))
      return opt;

    if ((opt = update_chunk(size)))
      return opt;
  }

  else {
    long pos = ftell(program.file);

    if ((opt = error_out_of_range(pos, size, 0, program.size)))
      return opt;

    if ((opt = file_at(pos + size)))
      return opt;
  }

  return OK;
}

/*******************************************************************************
                             NEXT
*/

int cmd_next(int argc, char **argv) {
  int opt;

  if ((opt = cmd_skip(argc, argv)))
    return opt;

  show_buffer_hex(chunk.data, chunk.used, chunk.offset);
  return OK;
}

/*******************************************************************************
                            TELL
*/

int cmd_tell(int argc, char **argv) {
  int opt;

  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  long pos = ftell(program.file);
  printf("current offset: %li (%lX)\n", pos, pos);
  return OK;
}

/*******************************************************************************
 *                          FINDNEXT
 */

int cmd_findnext(int argc, char **argv) {
  int opt, valid;
  long size;

  if ((opt = error_invalid_arguments(argc, 3)))
    return opt;

  valid = str_is_int(argv[1], &size);
  if ((opt = error_invalid_integer(valid, argv[1])))
    return opt;

  char *text = clistr_to_cstr(argv[2], len);

  return OK;
}

/*******************************************************************************
                            Command dispatcher
*/

void exec(const char *cmd, int argc, char **argv) {
  typedef struct cmd_map {
    const char *cmd;
    int (*func)(int, char **);
    const char *desc;
  } cmd_map;

  static const cmd_map map[] = {
      (cmd_map){.cmd = "open", .func = cmd_open, .desc = "open P file."},
      (cmd_map){.cmd = "close", .func = cmd_close, .desc = "close file."},
      (cmd_map){.cmd = "skip", .func = cmd_skip, .desc = "skip N bytes."},
      (cmd_map){.cmd = "next", .func = cmd_next, .desc = "show next N bytes."},
      (cmd_map){.cmd = "set", .func = cmd_set, .desc = "set offset to X."},
      (cmd_map){.cmd = "tell", .func = cmd_tell, .desc = "show current offset."}
      /**/
  };

  size_t num = sizeof(map) / sizeof(map[0]);

  if (strcmp(cmd, "help") == 0) {
    for (size_t i = 0; i < num; i++) {
      printf("%-10s %s\n", map[i].cmd, map[i].desc);
    }
    puts("Note:\n"
         "P = argument is a path\n"
         "N = argument is a number\n"
         "X = argument is a number\n");
    return;
  }

  for (size_t i = 0; i < num; i++) {
    if (strcmp(map[i].cmd, cmd) == 0) {
      int opt;
      if ((opt = map[i].func(argc, argv)))
        printf("error: command %s failed (return code %i).\n", cmd, opt);
      else
        printf("info: command %s succeeded.\n", cmd);
      return;
    }
  }

  printf("error: unrecognised command %s.\n", cmd);
  return;
}

/*******************************************************************************
                             Command functions
********************************************************************************/

#define DEFAULT_CMD_RESULT -1
int main(int margc, char **margv) {
  char buffer[INPUT_BUFFER];
  int quit = 0;

  while (quit == 0) {
    int argc = 0;
    char **argv = NULL;

    printf("command > ");

    argv = parse_input(buffer, INPUT_BUFFER, &argc);
    if (argv == NULL || argc == 0)
      continue;

    if (strcmp(argv[0], "quit") == 0)
      quit = 1;
    else
      exec(argv[0], argc, argv);

    free(argv);
  }

  if (program.file != NULL)
    close_file();

  return 0;
}
