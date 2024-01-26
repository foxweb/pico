#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <stdio.h>
#include <string.h>

// Client request
extern char *methodstr, // "GET" or "POST"
    *uri,               // "/index.html" things before '?'
    *querystr,          // "a=1&b=2" things after  '?'
    *prot,              // "HTTP/1.1"
    *payload;           // for POST

extern int payload_size;

// Server control functions
void serve_forever(const char *PORT);

char *request_header(const char *name);

typedef struct {
  char *name, *value;
} header_t;

header_t *request_headers(void);

#define METHOD_NONE 0
#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_HEAD 3
#define METHOD_DELETE 4
#define METHOD_OPTIONS 5
#define METHOD_PUT 6
#define METHOD_TRACE 7

#define METHOD_STR(m)                                                          \
  "NONE\0   "                                                                  \
  "GET\0    "                                                                  \
  "POST\0   "                                                                  \
  "HEAD\0   "                                                                  \
  "DELETE\0 "                                                                  \
  "OPTIONS\0"                                                                  \
  "PUT\0    "                                                                  \
  "TRACE\0  " +                                                                \
      (((m)&0x07) * 8)

extern int method;

// user shall implement this function

void route();

// Response
#define RESPONSE_PROTOCOL "HTTP/1.1"

#define HTTP_200 printf("%s 200 OK\r\n", RESPONSE_PROTOCOL)
#define HTTP_201 printf("%s 201 Created\r\n", RESPONSE_PROTOCOL)
#define HTTP_400 printf("%s 400 Bad Request\r\n", RESPONSE_PROTOCOL)
#define HTTP_404 printf("%s 404 Not found\r\n", RESPONSE_PROTOCOL)
#define HTTP_500 printf("%s 500 Internal Server Error\r\n", RESPONSE_PROTOCOL)

// some interesting macro for `route()`
#define ROUTE_START() if (0) {
#define ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if ((METHOD == method) && strcmp(URI, uri) == 0) {
#define GET(URI) ROUTE(METHOD_GET, URI)
#define POST(URI) ROUTE(METHOD_POST, URI)
#define HEAD(URI) ROUTE(METHOD_HEAD, URI)
#define ROUTE_END()                                                            \
  }                                                                            \
  else HTTP_500;

#endif
