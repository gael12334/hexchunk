/*
 * Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
 */

#include "../stream.h"
#include "../test.h"

/*******************************************************************************
 *                            Test data
 *******************************************************************************/

char s_data[] = "hello world!";
char s_area10[10];
char s_area20[20];
sb_t s_databuf = s_array(s_data);
sb_t s_area10buf = s_array(s_area10);
sb_t s_area20buf = s_array(s_area20);
const char s_path[] = "dummy.txt";

/*******************************************************************************
 *                       Test utility functions
 *******************************************************************************/

long s_util_file_size(FILE *file) {
  fpos_t pos;
  fgetpos(file, &pos);
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fsetpos(file, &pos);
  return size;
}

stream_t s_util_init_stream(FILE *h, long s, sm_t m, st_t t) {
  return (stream_t){.handle = h, .mode = m, .type = t, .size = s};
}

void s_util_create_file() {
  FILE *file = fopen(s_path, "w");
  assert(file != NULL);
  fwrite(s_data, 1, sizeof(s_data), file);
  fclose(file);
}

stream_t s_util_open_RF(void) {
  FILE *file = fopen(s_path, "r");
  assert(file != NULL);
  long size = s_util_file_size(file);
  return s_util_init_stream(file, size, sm_read, st_file);
}

stream_t s_util_open_AF(void) {
  FILE *file = fopen(s_path, "a");
  assert(file != NULL);
  fwrite(s_data, 1, sizeof(s_data), file);
  long size = s_util_file_size(file);
  return s_util_init_stream(file, size, sm_append, st_file);
}

stream_t s_util_open_WpF(void) {
  FILE *file = fopen(s_path, "w+");
  assert(file != NULL);
  long size = s_util_file_size(file);
  return s_util_init_stream(file, size, sm_writeplus, st_file);
}

stream_t s_util_open_RM(void) {
  long size = sizeof(s_data);
  FILE *memory = fmemopen(s_data, size, "r");
  assert(memory != NULL);
  return s_util_init_stream(memory, size, sm_read, st_file);
}

stream_t s_util_open_WpM(void) {
  FILE *memory = fmemopen(s_data, sizeof(s_data), "w+");
  assert(memory != NULL);
  return s_util_init_stream(memory, 0L, sm_writeplus, st_file);
}

/*******************************************************************************
 *                           Test cases
 *******************************************************************************/

void s_test_openfile_write(void) {
  // arrange
  stream_t stream;
  se_t error;

  // act
  error = s_openfile(&stream, "dummy.txt", sm_write);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_nexp("%p", NULL, "%p", stream.handle, {});
  t_exp("%li", 0L, "%li", stream.size, { fclose(stream.handle); });
  t_exp("%li", st_file, "%li", stream.type, { fclose(stream.handle); });
  t_exp("%li", sm_write, "%li", stream.mode, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_openfile_read(void) {
  // arrange
  s_util_create_file();
  const long size = sizeof(s_data);
  stream_t stream;
  se_t error;

  // act
  error = s_openfile(&stream, "dummy.txt", sm_read);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_nexp("%p", NULL, "%p", stream.handle, {});
  t_exp("%li", size, "%li", stream.size, { fclose(stream.handle); });
  t_exp("%li", st_file, "%li", stream.type, { fclose(stream.handle); });
  t_exp("%li", sm_read, "%li", stream.mode, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_openfile_writeplus(void) {
  /* Notes:
   * When opening with W, W+ or Wb+, length is truncated to zero.
   * https://www.manpagez.com/man/3/fopen/
   */

  // arrange
  s_util_create_file();
  stream_t stream;
  se_t error;

  // act
  error = s_openfile(&stream, "dummy.txt", sm_writeplus);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_nexp("%p", NULL, "%p", stream.handle, {});
  t_exp("%li", 0L, "%li", stream.size, { fclose(stream.handle); });
  t_exp("%li", st_file, "%li", stream.type, { fclose(stream.handle); });
  t_exp("%li", sm_writeplus, "%li", stream.mode, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_openmem(void) {
  // arrange
  stream_t stream;
  size_t size = sizeof(s_data);
  se_t error;

  // act
  error = s_openmem(&stream, &s_databuf, sm_read);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_nexp("%p", NULL, "%p", stream.handle, {});
  t_exp("%li", size, "%li", stream.size, { fclose(stream.handle); });
  t_exp("%li", st_memory, "%li", stream.type, { fclose(stream.handle); });
  t_exp("%li", sm_read, "%li", stream.mode, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_close(void) {
  // arrange
  stream_t stream = s_util_open_RF();
  se_t error;

  // act
  error = s_close(&stream);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_exp("%p", NULL, "%p", stream.handle, { fclose(stream.handle); });
  t_exp("%li", (long)0, "%li", stream.size, {});
  t_exp("%li", (long)0, "%li", stream.type, {});
  t_exp("%li", (long)0, "%li", stream.mode, {});
  t_ok();
}

void s_test_flush(void) {
  // arrange
  stream_t stream = s_util_open_AF();
  se_t error;

  // act
  error = s_flush(&stream);

  // assert
  t_exp("%i", se_ok, "%i", error, {
    if (stream.handle)
      fclose(stream.handle);
  });
  t_ok();

  fclose(stream.handle);
}

void s_test_length_memory_read(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long size = s_util_file_size(stream.handle);
  long length = 0;
  se_t error;

  // act
  error = s_length(&stream, &length);

  // assert
  t_exp("%i", se_ok, "%i", error, { fclose(stream.handle); });
  t_exp("%li", size, "%li", length, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_length_memory_write(void) {
  // arrange
  stream_t stream = s_util_open_WpM();
  long length = 0;
  se_t error;

  // act
  error = s_length(&stream, &length);

  // assert
  t_exp("%i", se_ok, "%i", error, { fclose(stream.handle); });
  t_exp("%li", 0L, "%li", length, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_length_file_append(void) {
  // arrange
  stream_t stream = s_util_open_AF();
  long size = stream.size;
  long length = 0;
  se_t error;

  // act
  error = s_length(&stream, &length);

  // assert
  t_exp("%i", se_ok, "%i", error, { fclose(stream.handle); });
  t_exp("%li", size, "%li", length, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_length_file_writeplus(void) {
  /* Notes:
   * When opening with W, W+ or Wb+, length is truncated to zero.
   * https://www.manpagez.com/man/3/fopen/
   */

  // arrange
  stream_t stream = s_util_open_WpF();
  long length = 0;
  se_t error;

  // act
  error = s_length(&stream, &length);

  // assert
  t_exp("%i", se_ok, "%i", error, { fclose(stream.handle); });
  t_exp("%li", 0L, "%li", length, { fclose(stream.handle); });
  t_ok();

  fclose(stream.handle);
}

void s_test_read(void) {
  // arrange
  s_util_create_file();
  stream_t stream = s_util_open_RF();
  long size = sizeof(s_area10);
  long read = 0;

  // act
  se_t error = s_read(&stream, &s_area10buf, &read);

  // assert
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", size, "%li", read, {});
  t_exp("%c", s_data[0], "%c", s_area10[0], {});
  t_ok();
}

void s_test_write(void) {
  // arrange
  stream_t stream = s_util_open_WpM();
  long size = sizeof(s_area10);
  long written = 0;

  memcpy(s_area10, "aaaaaaaaa", size);

  // act
  se_t error = s_write(&stream, &s_area10buf, &written);

  // assert
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", size, "%li", written, {});
  t_exp("%c", s_area10[0], "%c", s_data[0], {});
  t_ok();
}

void s_test_poll(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long input_size = 30;
  long expected = input_size - stream.size;
  long excess = 0;

  // act
  se_t error = s_poll(&stream, input_size, &excess);

  // assert
  fclose(stream.handle);
  t_exp("%i", se_size, "%i", error, {});
  t_exp("%li", expected, "%li", excess, {});
  t_ok();
}

void s_test_start(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  fseek(stream.handle, 0, SEEK_END);

  // act
  se_t error = s_start(&stream);

  // assert
  long pos = ftell(stream.handle);
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", 0L, "%li", pos, {});
  t_ok();
}

void s_test_end(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long size = stream.size;

  // act
  se_t error = s_end(&stream);

  // assert
  long pos = ftell(stream.handle);
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", size, "%li", pos, {});
  t_ok();
}

void s_test_pos(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long pos = 5;
  long out = 0;
  fseek(stream.handle, pos, SEEK_SET);

  // act
  se_t error = s_pos(&stream, &out);

  // assert
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", pos, "%li", out, {});
  t_ok();
}

void s_test_move(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long where = 5;

  // act
  se_t error = s_move(&stream, where);

  // assert
  long pos = ftell(stream.handle);
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", where, "%li", pos, {});
  t_ok();
}

void s_test_push(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long inital_off = 2;
  long push_off = 5;
  long where = inital_off + push_off;
  fseek(stream.handle, inital_off, SEEK_SET);

  // act
  se_t error = s_push(&stream, push_off);

  // assert
  long pos = ftell(stream.handle);
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", where, "%li", pos, {});
  t_ok();
}

void s_test_pop(void) {
  // arrange
  stream_t stream = s_util_open_RM();
  long inital_off = 7;
  long pop_off = 2;
  long where = inital_off - pop_off;
  fseek(stream.handle, inital_off, SEEK_SET);

  // act
  se_t error = s_pop(&stream, pop_off);

  // assert
  long pos = ftell(stream.handle);
  fclose(stream.handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", where, "%li", pos, {});
  t_ok();
}

void s_test_seek(void) {
  // arrange
  strcpy(s_data, "hello world");
  stream_t stream = s_util_open_RM();
  long which;
  sb_t needles[2];
  needles[0] = (sb_t){.data = "allo", .size = 4};
  needles[1] = (sb_t){.data = "world", .size = 5};
  stream_t list[2];
  s_openmem(&list[0], &needles[0], sm_read);
  s_openmem(&list[1], &needles[1], sm_read);

  // act
  se_t error = s_seek(&stream, list, 2, &which);

  // assert
  fclose(stream.handle);
  fclose(list[0].handle);
  fclose(list[1].handle);
  t_exp("%i", se_ok, "%i", error, {});
  t_exp("%li", 1L, "%li", which, {});
  t_ok();
}

int main(int argc, char **argv) {
  s_test_openfile_write();
  s_test_openfile_read();
  s_test_openfile_writeplus();
  s_test_openmem();
  s_test_close();
  s_test_flush();
  s_test_length_memory_read();
  s_test_length_memory_write();
  s_test_length_file_append();
  s_test_length_file_writeplus();
  s_test_read();
  s_test_write();
  s_test_poll();
  s_test_start();
  s_test_end();
  s_test_pos();
  s_test_move();
  s_test_push();
  s_test_pop();
  s_test_seek();
  return 0;
}
