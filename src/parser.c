/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "parser.h"

/*******************************************************************************
 *                                Constants
 *******************************************************************************/

#define p_digit "0123456789"
#define p_uppercase "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define p_lowercase "abcdefghijklmnopqrstuvwxyz"
#define p_xdigit "0123456789ABCDEFabcdef"
#define p_space "\t\n\v\f\r "
#define p_alphanum p_digit p_uppercase p_lowercase

/*******************************************************************************
 *                        Constants for hexadecimal parsing
 *******************************************************************************/

#define p_hexch_type(ch) ((((ch) & 0b00110000) >> 5) - 1)
const char p_hexch_offset[] = {0x30, 0x31, 0x51};
const int64_t p_hexch_bits[] = {0b0000001111111111, 0b1111110000000000,
                                0b1111110000000000};

/*******************************************************************************
 *                              Raise error macro
 *******************************************************************************/

#define p_raise_empty_text_error(text, start) /*  */                                     \
  e_throw(start - text, "Empty text fault. ")

#define p_raise_unexpected_char(text, stop)                                              \
  e_throw(stop - text, "Unexpected char fault. ")

#define p_raise_integer_overflow(text, stop)                                             \
  e_throw(stop - text, "Integer overflow fault. ")

#define p_raise_missing_closing_dquotes(text, stop)                                      \
  e_throw(stop - text, "Missing closing double-quotes fault. ")

/*******************************************************************************
 *                                Implementation
 *******************************************************************************/

int p_parse_integer(char* text, char** end, size_t n, int64_t* i64) {
  if (text == NULL)
    e_throw_NULPTR(text);

  if (end == NULL)
    e_throw_NULPTR(end);

  if (i64 == NULL)
    e_throw_NULPTR(i64);

  // string-end NUL-character safety
  text[n - 1] = '\0';
  char* start = text + strspn(text, p_space);

  // check if number is negative
  int64_t sign = 1;
  if (*start == '-') {
    sign = -1;
    start++;
  }

  char* stop = start + strspn(start, p_digit);

  // empty string
  if (start == stop)
    e_throw_UNKERR(); // p_raise_empty_text_error(text, start);

  // if not a space nor NUL, then unexpected char
  if (!isspace(*stop) && *stop != '\0')
    e_throw_UNKERR(); // p_raise_unexpected_char(text, stop);

  (*i64) = 0;
  for (char* d = stop; d != start; d--) {
    int64_t next = (*i64) + (d[-1] - '0') * 10; // I know, d[-1] is cursed to look at
    if (next < (*i64))
      e_throw_UNKERR(); // p_raise_integer_overflow(text, d);
    (*i64) = next;
  }

  (*i64) *= sign;
  (*end) = stop;
  return 0;
}

int p_parse_hexint(char* text, char** end, size_t n, int64_t* i64) {
  int error;
  char* begin;
  size_t size;
  int8_t* bytes = (int8_t*)i64;
  size_t bytesz = sizeof(*i64);

  e_trycatch(p_parse_hexstr(text, &begin, end, n, &size), { e_throw_THROWN(); });

  if (size > bytesz)
    e_throw_UNKERR();

  e_trycatch(p_parse_hexadecimal(begin, *end, bytes, bytesz), { e_throw_THROWN(); });

  return 0;
}

int p_parse_float(char* text, char** end, size_t n, double* f64) {
  e_throw_NOTIMP();
}

int p_parse_keyword(char* text, char** begin, char** end, size_t n) {
  if (text == NULL)
    e_throw_NULPTR(text);

  if (begin == NULL)
    e_throw_NULPTR(begin);

  if (end == NULL)
    e_throw_NULPTR(end);

  // string-end NUL-character safety
  text[n - 1] = '\0';
  char* start = text + strspn(text, p_space);
  char* stop = start + strspn(text, p_alphanum);

  // empty string
  if (start == stop)
    e_throw_UNKERR(); // p_raise_empty_text_error(text, start);

  // if not a space nor NUL, then unexpected char
  if (!isspace(*stop) && *stop != '\0')
    e_throw_UNKERR(); // p_raise_unexpected_char(text, stop);

  *begin = start;
  *end = stop;
  return 0;
}

int p_parse_string(char* text, char** begin, char** end, size_t n) {
  if (text == NULL)
    e_throw_NULPTR(text);

  if (begin == NULL)
    e_throw_NULPTR(begin);

  if (end == NULL)
    e_throw_NULPTR(end);

  // string-end NUL-character safety
  text[n - 1] = '\0';
  char* start = text + strspn(text, p_space);

  // a string starting without a double-quote is a keyword: a sequence of chars consisted
  // of only alphanumerical chars, spaces excluded.
  if (*start != '\"') {
    e_trycatch(p_parse_keyword(text, begin, end, n), { e_throw_THROWN(); });
    return 0;
  }

  // string literal requires to have a closing double-quote. Double-quotes are prohibited
  // inside a literal.
  start++;
  char* stop = strchr(start, '\"');
  if (stop == NULL)
    e_throw_UNKERR(); // p_raise_missing_closing_dquotes(text, stop);

  if (!isspace(stop[1]) && stop[1] != '\0')
    e_throw_UNKERR(); // p_raise_unexpected_char(text, stop + 1);

  *begin = start;
  *end = stop;
  return 0;
}

int p_parse_hexstr(char* text, char** begin, char** end, size_t n, size_t* size) {
  if (text == NULL)
    e_throw_NULPTR(text);

  if (begin == NULL)
    e_throw_NULPTR(begin);

  if (end == NULL)
    e_throw_NULPTR(end);

  // string-end NUL-character safety
  text[n - 1] = '\0';
  char* start = text + strspn(text, p_space);
  char* stop = start + strspn(text, p_xdigit);

  // empty string
  if (start == stop)
    e_throw_UNKERR(); // p_raise_empty_text_error(text, start);

  // if not a space nor NUL, then unexpected char
  if (!isspace(*stop) && *stop != '\0')
    e_throw_UNKERR(); // p_raise_unexpected_char(text, stop);

  *begin = start;
  *end = stop;
  *size = (stop - start);
  *size += (*size) & 1;
  *size >>= 1;
  return 0;
}

int p_parse_hexadecimal(char* begin, char* end, int8_t* bytes, size_t n_bytes) {
  if (begin == NULL)
    e_throw_NULPTR(begin);

  if (end == NULL)
    e_throw_NULPTR(end);

  if (bytes == NULL)
    e_throw_NULPTR(bytes);

  size_t length;

  // checking sizes
  length = end - begin;
  length += (length & 1);
  length >>= 1;
  if (n_bytes != length)
    e_throw_UNKERR(); // e_raise(1, "Invalid buffer size fault.");

  length = end - begin;
  char ch = 0;
  char er = 0;
  char hb = 0;
  char xx = 0;
  char mk = 0;
  char hi = (length & 1);
  bytes[0] = 0;

  int64_t off[] = {0x30, 0x31, 0x51};
  int64_t bit[] = {0x03FF, 0x0FC0, 0x0FC0};
  int64_t index = 0;
  int64_t write = 0;

  while (index < length && er == 0) {
    // type
    ch = begin[index];
    ch &= 0b00110000;
    ch >>= 4;
    ch -= 1;

    // half-byte value
    hb = begin[index];
    hb -= off[ch];

    // inverts last 2 bits of half-byte
    xx = hb;
    xx &= 0xC0;
    xx >>= 6;
    hb &= 0x3F;

    // validation
    er = bit[ch];
    er >>= hb;
    er = ~er;
    er |= xx;
    xx >>= 1;
    er |= xx;
    er &= 1;

    // set
    hi <<= 2;
    hb <<= hi;
    hi >>= 2;
    bytes[write] *= hi;
    bytes[write] |= hb;
    write += hb;
    hb = ~hb;
    hb &= 1;
    index++;
  }

  if (er > 0)
    e_throw_UNKERR(); // p_raise_unexpected_char(begin, begin + index);

  return 0;
}

int p_parse_any(char* text, char** begin, char** end, size_t n, size_t* size) {
  if (text == NULL)
    e_throw_NULPTR(text);

  if (begin == NULL)
    e_throw_NULPTR(begin);

  if (end == NULL)
    e_throw_NULPTR(end);

  // string-end NUL-character safety
  text[n - 1] = '\0';
  char* start = text + strspn(text, p_space);
  char* stop = text + strcspn(text, p_space);

  *begin = start;
  *end = stop;
  return 0;
}
