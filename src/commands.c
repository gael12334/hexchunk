/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "commands.h"

#define c_whitespaces " \t\r\v\f\n"

/*******************************************************************************
 *                               Internal functions
 *******************************************************************************/

int c_gettok(const charview_t in, charview_t* out) {
  size_t in_ndx = 0;
  while (in_ndx < in.count && isspace(in.address[in_ndx])) {
    in_ndx++;
  }

  // abnormal state -- segmentation fault
  if (in_ndx > in.count) {
    e_throw_SIGSEG(in.address + in_ndx);
  }

  // empty string
  if (in_ndx == in.count) {
    out->count = 0;
    out->address = in.address + in_ndx;
    return __OK__;
  }

  size_t out_sz = 0;

  // parsing string literal
  if (in.address[in_ndx] == '\"') {
    in_ndx++;
    while (in_ndx < in.count && in.address[in_ndx] != '\"') {
      in_ndx++;
      out_sz++;
    }

    // abnormal state -- segmentation fault
    if (in_ndx > in.count) {
      e_throw_SIGSEG(in.address + in_ndx);
    }

    // missing closing double quotes
    if (in_ndx == in.count) {
      e_throw_BADOBJ(in);
    }

    // expect to have whitespace after closing double-quote
    if (in_ndx + 1 < in.count && !isspace(in.address[in_ndx + 1])) {
      e_throw_BADOBJ(in);
    }

    in_ndx++;
  }

  else {
    while (in_ndx < in.count && !isspace(in.address[in_ndx])) {
      in_ndx++;
      out_sz++;
    }

    // abnormal state -- segmentation fault
    if (in_ndx > in.count) {
      e_throw_SIGSEG(in.address + in_ndx);
    }
  }

  out->address = in.address + in_ndx;
  out->count = out_sz;
  return __OK__;
}

int c_cmptok(charview_t f, charview_t i, arrayview_t a, arrayview_t v) {
  size_t arg_ndx = 0;
  arrayviewof_t(catype_t) arg_arr = avof_cast(&a);

  while (f.count > 0 && i.count > 0) {
    e_trycatch(c_gettok(f, &f), { e_throw_THROWN(); });
    e_trycatch(c_gettok(i, &i), { e_throw_THROWN(); });

    if (f.address[0] == '*') {
      switch (arg_arr.address[0]) { case CARGINT: }
    }
  }
}

/*******************************************************************************
 *                                 Implementation
 *******************************************************************************/

int c_init(cmdmgr_t* mgr) {
  if (mgr == NULL)
    e_throw_NULPTR(mgr);

  e_trycatch(lvof_resize(&mgr->commands, c_default_alloc), e_throw_THROWN());
  return __OK__;
}

int c_deinit(cmdmgr_t* mgr) {
  if (mgr == NULL)
    e_throw_NULPTR(mgr);

  e_trycatch(lvof_resize(&mgr->commands, 0), e_throw_THROWN());
  return __OK__;
}

int c_register(cmdmgr_t* mgr, charview_t pattern, vaargs_t argstype, cmdfn_t handler) {
  if (mgr == NULL)
    e_throw_NULPTR(mgr);

  if (handler == NULL)
    e_throw_NULPTR(handler);

  if (pattern.address == NULL)
    e_throw_BADOBJ(pattern.address);

  if (argstype.address == NULL)
    e_throw_BADOBJ(argstype.address);

  cmd_t cmd;
  cmd.pattern = pattern;
  cmd.cargstype.address = argstype.address;
  cmd.cargstype.count = argstype.count;
  cmd.handler = handler;
  e_trycatch(avof_alloc(&cmd.carguments, argstype.count), e_throw_THROWN());
  e_trycatch(lvof_append(&mgr->commands, &cmd), {
    avof_free(&cmd.carguments);
    e_throw_THROWN();
  });

  return __OK__;
}

int c_find(cmdmgr_t* mgr, charview_t pattern, cmd_t* out) {
  if (mgr == NULL)
    e_throw_NULPTR(mgr);

  if (out == NULL)
    e_throw_NULPTR(out);

  if (pattern.address == NULL)
    e_throw_BADOBJ(pattern.address);

  cmd_t* cmd;
  lvof_foreach(cmd, &mgr->commands) {
  }

  return __OK__;
}

int c_translate(cmd_t* cmd) {
  return __OK__;
}

int c_execute(cmd_t* cmd) {
  return __OK__;
}
