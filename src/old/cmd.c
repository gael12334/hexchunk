/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "error.h"
#include <stdio.h>

/*******************************************************************************
 *                            Command functions
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
 *                            CLOSE
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
 *                            SET
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
 *                            SKIP
 */

// TODO reimplement
int cmd_skip(int argc, char **argv) {
  // long size = CHUNK;
  // int opt, valid;
  //
  // if (argc >= 2) {
  //   if ((opt = error_invalid_arguments(argc, 2)))
  //     return opt;
  //
  //   valid = str_is_int(argv[1], &size);
  //   if ((opt = error_invalid_integer(valid, argv[1])))
  //     return opt;
  // }
  //
  // if (strcmp(argv[0], "next") == 0) {
  //   if ((opt = error_out_of_range(0, size, -CHUNK, CHUNK)))
  //     return opt;
  //
  //   if ((opt = next_chunk(size)))
  //     return opt;
  // }
  //
  // else if (strcmp(argv[0], "prev") == 0) {
  //   if ((opt = error_out_of_range(0, size, -CHUNK, CHUNK)))
  //     return opt;
  //
  //   if ((opt = next_chunk(-size)))
  //     return opt;
  // }
  //
  // else {
  //   long pos = ftell(program.file);
  //
  //   if ((opt = error_out_of_range(pos, size, 0, program.size)))
  //     return opt;
  //
  //   if ((opt = file_at(pos + size)))
  //     return opt;
  // }

  return OK;
}

/*******************************************************************************
 *                            NEXT
 */

int cmd_next(int argc, char **argv) {
  int opt;

  if ((opt = cmd_skip(argc, argv)))
    return opt;

  bytebuf_hexview(chunk.data, chunk.used, chunk.offset);
  return OK;
}

/*******************************************************************************
 *                            PREV
 */

int cmd_prev(int argc, char **argv) {
  int opt;

  if ((opt = cmd_skip(argc, argv)))
    return opt;

  bytebuf_hexview(chunk.data, chunk.used, chunk.offset);
  return OK;
}

/*******************************************************************************
 *                           TELL
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

  return scan_many_chunk(argv[2], size);
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
  return scan_many_chunk(args[1], 0);
}

/*******************************************************************************
 *                           Command dispatcher
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

void entry(void) {
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
}
