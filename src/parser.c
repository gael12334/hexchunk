/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "parser.h"
#define p_cvspaces " \t\f\v\n\r"

static int p_parse_long(charview_t token, value_t* value, int* result) {
  if (token.address == NULL)
    e_throw_BADOBJ(token);

  if (value == NULL)
    e_throw_NULPTR(value);

  // string-end NUL-character safety
  char back;
  back = token.address[token.count - 1];
  token.address[token.count - 1] = '\0';

  // convert string to long
  char* end;
  long integer = strtol(token.address, &end, 10);
  if (end == token.address)
    goto throw_bad_obj;
  else if (*end != '\0')
    goto throw_bad_obj;
  else if (errno == ERANGE)
    goto throw_bad_obj;

  token.address[token.count - 1] = back;
  value->poly.i64 = integer;
  value->size = sizeof(integer);
  value->type = v_int;
  return __OK__;

  // throws bad obj
throw_bad_obj:
  token.address[token.count - 1] = back;
  e_throw_BADOBJ(token);
}

static int p_parse_real(charview_t token, value_t* value, int* result) {
  if (token.address == NULL)
    e_throw_BADOBJ(token);

  if (value == NULL)
    e_throw_NULPTR(value);

  // string-end NUL-character safety
  char back;
  back = token.address[token.count - 1];
  token.address[token.count - 1] = '\0';

  // convert string to real
  char* end;
  double real = strtod(token.address, &end);
  if (end == token.address)
    goto throw_bad_obj;
  else if (*end != '\0')
    goto throw_bad_obj;
  else if (errno == ERANGE)
    goto throw_bad_obj;

  token.address[token.count - 1] = back;
  value->poly.f64 = real;
  value->size = sizeof(real);
  value->type = v_num;
  return __OK__;

  // throws bad obj
throw_bad_obj:
  token.address[token.count - 1] = back;
  e_throw_BADOBJ(token);
}

static int p_parse_text(charview_t token, value_t* value, int* result) {
  if (token.address == NULL)
    e_throw_BADOBJ(token);

  if (value == NULL)
    e_throw_NULPTR(value);

  if (token.address[0] == '\"' && token.address[token.count - 1] == '\"') {
    value->size = token.count;
    value->poly.str = token.address;
    value->type = v_str;
    return __OK__;
  }

  e_throw_BADOBJ(token);
}

static int p_parse_hexa(charview_t token, value_t* value, int* result) {
  if (token.address == NULL)
    e_throw_BADOBJ(token);

  if (value == NULL)
    e_throw_NULPTR(value);

  // string-end NUL-character safety
  char back;
  back = token.address[token.count - 1];
  token.address[token.count - 1] = '\0';

  // convert hex string to buffer of bytes
  char halves[3] = {0};
  size_t odd = (token.count - 1) & 1;
  size_t buffsz = token.count - 1 + odd;
  uint8_t* buffer = malloc(buffsz);
  if (buffer == NULL)
    e_throw_MEMALL();

  size_t b = 0;
  for (size_t i = odd; i < (token.count - 1) + odd; i++) {
    halves[i] = token.address[i];
    if (i & 1) {
      char* end;
      unsigned long integer = strtoul(halves, &end, 16);
      if (end == token.address)
        goto throw_bad_obj;
      else if (*end != '\0')
        goto throw_bad_obj;
      else if (errno == ERANGE)
        goto throw_bad_obj;

      memset(halves, 0, sizeof(halves));
      buffer[b] = (uint8_t)(integer & 0xFF);
      b++;
    }
  }

  token.address[token.count - 1] = back;
  value->poly.buf = buffer;
  value->size = buffsz;
  value->type = v_int;
  return __OK__;

// throws bad obj
throw_bad_obj:
  token.address[token.count - 1] = back;
  e_throw_BADOBJ(token);
}

static const clipfn_t parsers[INT8_MAX + 1] = {
    ['i'] = p_parse_long,
    ['f'] = p_parse_real,
    ['s'] = p_parse_text,
    ['x'] = p_parse_hexa,
};

static int p_wildcard(clip_t* clip, charview_t format, size_t tokndx) {
  if (format.count != 3) {
    e_throw_BADOBJ(format);
  }

  char* type = strchr("ifsx", format.address[1]);
  if (type == NULL) {
    e_throw_BADOBJ(format);
  }

  clip->pmask |= 1 << tokndx;
  clip->pnmbr++;
  clip->tokens[tokndx].token = format;
  clip->tokens[tokndx].parser = parsers[*type];
  return __OK__;
}

int p_init(clip_t* clip, charview_t format) {
  if (clip == NULL)
    e_throw_NULPTR(clip);

  if (format.address == NULL)
    e_throw_BADOBJ(format);

  char back;
  size_t tokndx = 0;
  size_t endndx = 0;
  size_t index = 0;

  *clip = (clip_t){0};
  back = format.address[format.count - 1];
  format.address[format.count - 1] = '\0';

  while (index < format.count && tokndx < p_toknb) {
    index += strspn(format.address + index, p_cvspaces);
    endndx = index + strcspn(format.address + index, p_cvspaces);

    charview_t subview = cvsub(format.address + index, endndx - index);
    switch (subview.address[0]) {
      case '*':
        e_trycatch(p_wildcard(clip, subview, tokndx), e_throw_THROWN());
        break;
      case '\"':
        e_throw_BADOBJ(format);
        break;
      default:
        clip->tokens[tokndx].token = subview;
        break;
    }
    index = endndx;
    tokndx++;
  }

  clip->toknsz = tokndx;
  format.address[format.count - 1] = back;
  return __OK__;
}

int p_deinit(clip_t* clip) {
  if (clip == NULL)
    e_throw_NULPTR(clip);

  memset(clip, 0, sizeof(*clip));
  return __OK__;
}

int p_match(clip_t* clip, charview_t input, int* result) {
  if (clip == NULL)
    e_throw_NULPTR(clip);
  if (input.address == NULL)
    e_throw_BADOBJ(format);
  if (result == NULL)
    e_throw_NULPTR(result);

  char back;
  size_t tokndx = 0;
  size_t endndx = 0;
  size_t index = 0;
  size_t tokchnb = 0;
  unsigned dquotes = 0;

  *clip = (clip_t){0};
  back = input.address[input.count - 1];
  input.address[input.count - 1] = '\0';

  while (index < input.count && tokndx < p_toknb) {
    index += strspn(input.address + index, p_cvspaces);
    dquotes = input.address[index] == '\"';
    tokchnb = strspn(input.address + index + dquotes, (dquotes) ? "\"" : p_cvspaces);
    dquotes = input.address[index + tokchnb] == '\"';
    tokchnb += dquotes;
    endndx = index + tokchnb;

    charview_t subview = cvsub(input.address + index, tokchnb);
    cliptok_t* token = &clip->tokens[tokndx];
    if (token->parser) {
      e_trycatch(token->parser(subview, &token->value, NULL), e_throw_THROWN());
    } /**/
    else if (token->token.count != tokchnb) {
      e_throw_BADOBJ(input);
    } /**/
    else if (memcpy(token->token.address, subview.address, tokchnb) != 0) {
      e_throw_BADOBJ(input);
    }

    index = endndx;
    tokndx++;
  }

  clip->toknsz = tokndx;
  input.address[input.count - 1] = back;
  return __OK__;
}

int p_param(clip_t* clip, size_t pndx, size_t* real_ndx) {
  if (clip == NULL)
    e_throw_NULPTR(clip);
  if (pndx >= clip->pnmbr)
    e_throw_SIGSEG(pndx);
  if (real_ndx == NULL)
    e_throw_NULPTR(real_ndx);

  size_t i = 0;
  size_t p = 0;
  while (i < pndx && i < p_toknb) {
    i += (clip->pmask >> p) & 1;
    p++;
  }

  if (clip->tokens[p].parser == NULL)
    e_throw_BADOBJ(clip);

  *real_ndx = p;
  return __OK__;
}
