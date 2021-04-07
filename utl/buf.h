#ifndef BUF_H___
#define BUF_H___

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct buf_s {
    char *buffer;
    int32_t size;
    int32_t count;
    int32_t pos;
} *buf_t;


buf_t   buf_new();
buf_t   buf_free(buf_t buf);
int32_t buf_size(buf_t buf);
int32_t buf_pos(buf_t buf);
int32_t buf_count(buf_t buf);
char   *buf_str(buf_t buf, int32_t pos);
int32_t buf_makeroom(buf_t buf, int32_t size);
int     buf_putc(buf_t buf,int c);
int32_t buf_printf(buf_t b, const char *fmt, ...);
int32_t buf_putc(buf_t buf, int c);
int32_t buf_puts(buf_t buf, char *src);
int32_t buf_write(buf_t buf, char *src, int32_t len);

#endif