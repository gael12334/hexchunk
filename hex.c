/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK 8192
#define OK 0
#define ERR 1

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
extern void bytebuf_hexview(uint8_t *buffer, long size, long start_at);
extern char **parse_input(char *buffer, size_t len, int *out);
extern int bytebuf_is_zeroed(uint8_t *buffer, long size);

/*******************************************************************************
                              Error functions
 ********************************************************************************/

extern int error_fopen(FILE *result, const char *path);
extern int error_out_of_range(long pos, long length, long low, long high);
extern int error_invalid_integer(int valid, const char *str);
extern int error_no_file_loaded(FILE *file);
extern int error_invalid_arguments(int got_argc, int expected_argc);
extern int error_not_enough_args(int got_argc, int expected_argc);

/*******************************************************************************
 *                            File functions
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

int create_file(const char *entered_path, FILE **file) {
  static char path[PATH_MAX];
  char choice = 0;
  FILE *output;

  realpath(entered_path, path);

  output = fopen(path, "r");
  if (output == NULL)
    goto newfile;
  else
    goto prompt;

prompt:
  fclose(output);

  while (choice != 'y' && choice != 'n') {
    printf("info: file %s already exists; overwrite? (y/n) > ", path);
    choice = getchar();
  }

  if (choice == 'n') {
    puts("info: scan aborted.");
    return ERR;
  }

  goto newfile;

newfile:
  *file = fopen(path, "w");
  return OK;
}

/*******************************************************************************
                             Chunk functions
********************************************************************************/

int update_chunk(long size) {
  int opt;
  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  long pos = ftell(program.file);
  if ((opt = error_out_of_range(pos, size, 0, program.size)))
    return opt;

  if (size < 0) {
    long new_pos = pos + size;
    long new_size = -size;

    fseek(program.file, new_pos, SEEK_SET);
    fread(chunk.data, new_size, 1, program.file);
    fseek(program.file, new_pos, SEEK_SET);
    chunk.used = new_size;
    chunk.offset = ftell(program.file);
    return OK;
  }

  fread(chunk.data, size, 1, program.file);
  chunk.used = size;
  chunk.offset = pos;
  return OK;
}

int scan_chunk(const char *entered_path, long chunks) {
  int opt;
  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  FILE *output;
  if ((opt = create_file(entered_path, &output)))
    return opt;

  long step;
  long limit;
  long chunk_count = 0;
  int chunk_empty = 1;
  int prev_empty = 2;
  long pos = ftell(program.file);

  chunks *= CHUNK;

  if (chunks == 0)
    limit = program.size;
  else if ((opt = error_out_of_range(pos, chunks, 0, program.size)))
    return opt;
  else
    limit = chunks;

  for (long i = ftell(program.file); i < program.size; i += CHUNK) {
    step = CHUNK;

    if (i + step > program.size)
      step = program.size - i - 1;

    if ((opt = update_chunk(step)))
      return opt;

    chunk_empty = bytebuf_is_zeroed(chunk.data, chunk.used);
    switch (prev_empty | chunk_empty) {
    case 0b01:
      fprintf(output, "%li chunks (%li)\n\n", chunk_count, chunk_count * CHUNK);
      chunk_count = 0;
      break;

    case 0b10:
      pos = ftell(program.file);
      fprintf(output, "%li (%lX)\n", pos, pos);
      chunk_count++;
      break;

    case 0b00:
      chunk_count++;
      break;
    }

    prev_empty = chunk_empty << 1;

    fflush(output);
  }

  fclose(output);
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
  long size = CHUNK;
  int opt, valid;

  if (argc >= 2) {
    if ((opt = error_invalid_arguments(argc, 2)))
      return opt;

    valid = str_is_int(argv[1], &size);
    if ((opt = error_invalid_integer(valid, argv[1])))
      return opt;
  }

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

  bytebuf_hexview(chunk.data, chunk.used, chunk.offset);
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
 *                          SCAN_NEXT
 */

int cmd_scan_next(int argc, char **argv) {
  int opt, valid;
  long size;

  if ((opt = error_invalid_arguments(argc, 3)))
    return opt;

  valid = str_is_int(argv[1], &size);
  if ((opt = error_invalid_integer(valid, argv[1])))
    return opt;

  return scan_chunk(argv[2], size);
}

/*******************************************************************************
 *                          SCAN
 */

int cmd_scan(int argc, char **args) {
  int opt;

  if ((opt = error_invalid_arguments(argc, 2)))
    return opt;

  if ((opt = error_no_file_loaded(program.file)))
    return opt;

  fseek(program.file, 0, SEEK_SET);
  return scan_chunk(args[1], 0);
}

/*******************************************************************************
                            Command dispatcher
*/

void exec(const char *cmd, int argc, char **argv) {
  typedef struct cmd_map {
    const char *cmd;
    int (*fn)(int, char **);
    const char *desc;
  } cmdmap;

  static const cmdmap map[] = {
      (cmdmap){.cmd = "open", .fn = cmd_open, .desc = "open P file."},
      (cmdmap){.cmd = "close", .fn = cmd_close, .desc = "close file."},
      (cmdmap){.cmd = "skip", .fn = cmd_skip, .desc = "skip N bytes."},
      (cmdmap){.cmd = "next", .fn = cmd_next, .desc = "show next N bytes."},
      (cmdmap){.cmd = "set", .fn = cmd_set, .desc = "set offset to X."},
      (cmdmap){.cmd = "tell", .fn = cmd_tell, .desc = "show offset."},
      (cmdmap){.cmd = "scan", .fn = cmd_scan, .desc = "scan for non-null data"}
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
      if ((opt = map[i].fn(argc, argv)))
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
  char buffer[1024];
  int quit = 0;

  while (quit == 0) {
    int argc = 0;
    char **argv = NULL;

    printf("command > ");

    argv = parse_input(buffer, sizeof(buffer) - 1, &argc);
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
