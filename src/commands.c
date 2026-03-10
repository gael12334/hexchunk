/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "commands.h"
#include "error.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 *                              Constants
 *******************************************************************************/

#define c_default_elem_num (16)
#define c_skip_command (0)
#define c_check_command (1)

/*******************************************************************************
 *                            Implementation
 *******************************************************************************/

int c_init(struct cmdmgr* cr) {
  if (cr == NULL)
    e_throw_NULPTR(cr);

  e_trycatch(lvof_resize(&cr->commands, c_default_elem_num), {/*no cleanup*/});
  return 0;
}

int c_deinit(struct cmdmgr* cr) {
  if (cr == NULL)
    e_throw_NULPTR(cr);

  // freeing all the command argument lists
  struct cmdobj* cmdobj;
  lvof_foreach(cmdobj, &cr->commands) {
    e_trycatch(lvof_resize(&cmdobj->args, 0), {/*no cleanup*/});
  }

  e_trycatch(lvof_resize(&cr->commands, 0), {/*no cleanup*/});
  return 0;
}

int c_subscribe(struct cmdmgr* cr, struct cmdobj* co) {
  if (cr == NULL)
    e_throw_NULPTR(cr);

  if (co == NULL)
    e_throw_NULPTR(co);

  e_trycatch(lvof_append(&cr->commands, co), {/*no cleanup*/});
  e_trycatch(lvof_resize(&co->args, c_default_elem_num), {/*no cleanup*/});
  co->reserved = 0;
  return 0;
}

int c_notify(struct cmdmgr* cr, char* command, size_t size) {
  if (cr == NULL)
    e_throw_NULPTR(cr);

  if (command == NULL)
    e_throw_NULPTR(command);

  char* text = command;
  char* tend = text + size;
  size_t fndx = 0;
  size_t left = cr->commands.count;

  while (left > 0) {
    struct cmdobj* co;
    lvof_foreach(co, &cr->commands) {
      if (fndx == 0)
        co->reserved = c_check_command;

      // format index beyond command format count.
      if (fndx >= co->length) {
        if (co->reserved == c_check_command)
          left--;
        continue;
      }

      // input was previously detected as invalid for this command
      if (co->reserved == c_skip_command)
        continue;

      struct cmdformat* format = &co->format[fndx];
      switch (format->type) {
        // absolute match
        case c_m: {
          char* begin;
          char* end;
          int parser_error = p_parse_keyword(text, &begin, &end, tend - text);
          int size_error = (end - begin != format->end - format->opt);

          if (parser_error || size_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          int match_error = strncmp(begin, format->opt, (end - begin));
          if (match_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          text = end;
          break;
        }

        // integer
        case c_i: {
          char* end;
          struct value value = {.type = v_int, .size = 0};
          int parser_error = p_parse_integer(text, &end, (tend - text), &value.cvu.i64);

          if (parser_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          e_trycatch(lvof_append(&co->args, &value), {});
          text = end;
          break;
        }

        // real number
        case c_f: {
          char* end;
          struct value value = {.type = v_num, .size = 0};
          int parser_error = p_parse_float(text, &end, (tend - text), &value.cvu.f64);

          if (parser_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          e_trycatch(lvof_append(&co->args, &value), {});
          text = end;
          break;
        }

        // string
        case c_s: {
          char* begin;
          char* end;
          size_t length = (tend - text);
          struct value value = {.type = v_str, .size = length};

          int parser_error = p_parse_string(text, &begin, &end, length);
          if (parser_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          int8_t* bytes = malloc(sizeof(char) * value.size);
          if (bytes == NULL)
            e_throw_NULPTR(bytes);

          memcpy(bytes, begin, length - 1);
          bytes[length - 1] = '\0';
          value.size = end - begin;
          value.cvu.str = (char*)bytes;
          e_trycatch(lvof_append(&co->args, &value), {});
          text = end;
          break;
        }

        // hex string
        case c_x: {
          char* begin;
          char* end;
          size_t length = (tend - text);
          struct value value = {.type = v_buf, .size = 0};

          int parser_error = p_parse_hexstr(text, &begin, &end, length, &value.size);
          if (parser_error) {
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          int8_t* bytes = malloc(sizeof(int8_t) * value.size);
          if (bytes == NULL)
            e_throw_NULPTR(bytes);

          int conversion_err = p_parse_hexadecimal(begin, end, bytes, value.size);
          if (conversion_err) {
            free(bytes);
            co->reserved = c_skip_command;
            left--;
            continue;
          }

          value.size = end - begin;
          value.cvu.buf = bytes;
          e_trycatch(lvof_append(&co->args, &value), {});
          text = end;

          break;
        }
      }
    }

    fndx++;
  }
  struct cmdobj* co;
  lvof_foreach(co, &cr->commands) {
    if (co->reserved == c_check_command)
      return co->handler(NULL, co->args.count, co->args.address);
  }

  return 1;
}
