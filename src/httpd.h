#ifndef HTTPD_H___
#define HTTPD_H___

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Client request
extern char *methodstr, // "GET" or "POST"
            *uri,            // "/index.html" things before '?'
            *querystr,             // "a=1&b=2" things after  '?'
            *prot,           // "HTTP/1.1"
            *payload;        // for POST

extern int payload_size;

// Server control functions
void  httpd_start(const char *PORT);
char *request_header(const char *name);

typedef struct {
  char *name, *value;
} header_t;

header_t *request_headers(void);

#define METHOD_NONE    0
#define METHOD_GET     1
#define METHOD_POST    2
#define METHOD_HEAD    3
#define METHOD_DELETE  4
#define METHOD_OPTIONS 5
#define METHOD_PUT     6
#define METHOD_TRACE   7

#define METHOD_STR(m)  "NONE\0   " \
                       "GET\0    " \
                       "POST\0   " \
                       "HEAD\0   " \
                       "DELETE\0 " \
                       "OPTIONS\0" \
                       "PUT\0    " \
                       "TRACE\0  " \
                     + (((m) & 0x07)*8)

extern int method;

typedef struct {
  char   *buffer;
  int32_t buffer_size;
  int32_t buffer_count;
  int32_t protocol_start;
  int32_t headers_start;
  int32_t querystr_start;
  int32_t payload_start;
  int32_t payload_size;
  int16_t headers_num;
  int16_t method;
} httpd_req_t;

char *httpd_header(httpd_req_t *req, char *hdr);
char *httpd_header_first(httpd_req_t *req);
char *httpd_header_next(httpd_req_t *req);
char *httpd_header_value(char *hdr);
int   httpd_header_count(httpd_req_t *req);
char *httpd_query(httpd_req_t *req);
char *httpd_queryarg_first(httpd_req_t *req);
char *httpd_payload(httpd_req_t *req);
char *httpd_protocol(httpd_req_t *req);
int   httpd_payload_size(httpd_req_t *req);
int   httpd_method(httpd_req_t *req);

// user shall implement the function
//  void httpd_route() 

// Thise one will be called by the listener upon
// successful request.

void httpd_route_();


// Response
#define RESPONSE_PROTOCOL "HTTP/1.1"

#define HTTP_200 printf("%s 200 OK\r\n", RESPONSE_PROTOCOL)
#define HTTP_201 printf("%s 201 Created\r\n", RESPONSE_PROTOCOL)
#define HTTP_400 printf("%s 400 Bad Request\r\n", RESPONSE_PROTOCOL)
#define HTTP_404 printf("%s 404 Not found\r\n", RESPONSE_PROTOCOL)
#define HTTP_500 printf("%s 500 Internal Server Error\r\n", RESPONSE_PROTOCOL)

#define HTTPD_RESP(n) 

#define HTTP_HDR(HDR,...) do {                   \
                            printf("%s: ",HDR);  \
                            printf(__VA_ARGS__); \
                            puts("\r\n");        \
                          } while(0)

// some interesting macro for `route()`
#define httpd_route() httpd_route_() { if (0) 

#define HTTPD_ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if ((METHOD == method) && strcmp(URI, uri) == 0 ) {

#define HTTPD_DEFAULT() } else

#define HTTPD_ERR()   } else HTTP_500

#define HTTPD_GET(URI)  HTTPD_ROUTE(METHOD_GET, URI)
#define HTTPD_POST(URI) HTTPD_ROUTE(METHOD_POST, URI)

#endif
