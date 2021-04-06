
#include "buf.h"

buf_t buf_new()
{
  buf_t buf;

  buf = malloc(sizeof(struct buf_s));
  if (buf) {
   buf->buffer = NULL;
   buf->size = 0;
   buf->count = 0;
   buf->pos = 0;
  }
  return buf;
}

buf_t buf_free(buf_t buf)
{
  if (buf) {
    free(buf->buffer);
    buf->buffer = NULL;
    buf->size = 0;
    buf->count = 0;
    buf->pos = 0;
    free(buf);
  }
  return NULL;
}

int32_t buf_size(buf_t buf)
{ return (buf?buf->size:0); }

int32_t buf_pos(buf_t buf)
{ return (buf?buf->pos:0); }

int32_t buf_count(buf_t buf)
{ return (buf?buf->count:0); }

char *buf_str(buf_t buf, int32_t pos)
{ 
  if (buf == NULL) return NULL;
  if (pos<0) pos = 0;
  else if (pos > buf->pos) pos= buf->pos;

  return buf->buffer + pos;
}

int32_t buf_makeroom(buf_t buf, int32_t size)
{
  int32_t new_size = 1;
  char *new_buffer = NULL;

  if (buf == NULL) return 0;
  if (size <= buf->size) return 1;
  new_size = buf->size ? buf->size : 1;
  while (new_size <= size) {
    new_size += (new_size / 2);  /* (new_size *= 1.5) */
    new_size += (new_size & 1);  /* ensure new size is even */
  }

  new_buffer = realloc(buf->buffer, new_size);
  if (new_buffer) { 
    buf->buffer = new_buffer;
    buf->size   = new_size;
    return 1;
  }

  errno = ENOMEM;
  return 0; 
}

int32_t buf_printf(buf_t buf, const char *fmt, ...)
{
  va_list args;
  int len,pos;
  char chr;

  pos = buf->pos;
  
  va_start(args, fmt);
  len = vsnprintf(NULL,0,fmt,args);
  va_end(args);
  
  if (len <= 0 || !buf_makeroom(buf,buf->pos+len+4)) return 0;
  chr = buf->buffer[buf->pos+len];

  va_start(args, fmt);
  len = vsnprintf(((char*)(buf->buffer))+pos,len+1,fmt,args);
  va_end(args);

  buf->buffer[buf->pos+len] = chr;
  buf->pos += len;
  if (buf->count < buf->pos) buf->count = buf->pos;
  buf->buffer[buf->count] = '\0';
  
  return len;
}

static int32_t buf_write_(buf_t buf, char* src, int32_t len, int32_t raw)
{
  if (!src || !buf) return 0;
  if (!raw && (len <= 0)) len = strlen(src);
  if (len <= 0) return 0;
  if (!buf_makeroom(buf,buf->pos+len+4)) return 0;
  int n;
  for (n = len; (raw || *src) && (n > 0); n--) {
    buf->buffer[buf->pos++] = *src++;
  }
  if (buf->count < buf->pos) buf->count = buf->pos;
  buf->buffer[buf->count] = '\0';
  return (len - n);
}

int32_t buf_putc(buf_t buf, int c)
{
  char ch = (char)c;
  return buf_write_(buf,&ch,1,1);
}

int32_t buf_puts(buf_t buf, char *src) 
{ return buf_write_(buf,src,0,0); }

int32_t buf_write(buf_t buf, char *src, int32_t len)
{ return buf_write_(buf,src,len,1); }
