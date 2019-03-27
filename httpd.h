#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

// Server control functions

void serve_forever(const char *PORT);

// Client request

char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2"     things after  '?'
    *prot;    // "HTTP/1.1"

char *payload; // for POST
int payload_size;

char *request_header(const char *name);

typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};
header_t *request_headers(void);

// user shall implement this function

void route();

// some interesting macro for `route()`
#define ROUTE_START() if (0) {
#define ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if (strcmp(URI, uri) == 0 && strcmp(METHOD, method) == 0) {
#define ROUTE_GET(URI) ROUTE("GET", URI)
#define ROUTE_POST(URI) ROUTE("POST", URI)
#define ROUTE_END()                                                            \
  }                                                                            \
  else printf("HTTP/1.1 500 Internal Server Error\n\n"                         \
              "The server has no handler to the request.\n");

#endif
