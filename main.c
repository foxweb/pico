#include "httpd.h"
#include <sys/stat.h>

#define RESPONSE_PROTOCOL "HTTP/1.1"

#define HTTP_200 printf("%s 200 OK\n\n", RESPONSE_PROTOCOL)
#define HTTP_201 printf("%s 201 Created\n\n", RESPONSE_PROTOCOL)
#define HTTP_404 printf("%s 404 Not found\n\n", RESPONSE_PROTOCOL)
#define HTTP_500 printf("%s 500 Internal Server Error\n\n", RESPONSE_PROTOCOL)

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR "./public"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

// Client request
extern char *method, // "GET" or "POST"
    *uri,            // "/index.html" things before '?'
    *qs,             // "a=1&b=2" things after  '?'
    *prot,           // "HTTP/1.1"
    *payload;        // for POST

extern int payload_size;

int main(int c, char **v) {
  char *port = c == 1 ? "8000" : v[1];
  serve_forever(port);
  return 0;
}

int file_exists(const char *file_name) {
  struct stat buffer;
  int exist = stat(file_name, &buffer);
  if (exist == 0)
    return 1;
  else
    return 0;
}

int read_file(const char *file_name) {
  char buf[CHUNK_SIZE];
  FILE *file;
  size_t nread;

  file = fopen(file_name, "r");

  if (file) {
    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
      fwrite(buf, 1, nread, stdout);

    if (ferror(file))  return 1;
    
    fclose(file);
    return 0;

  }
  return 1;

}

void route() {
  ROUTE_START()

  GET("/") {
    char index_html[20];
    sprintf(index_html, "%s%s", PUBLIC_DIR, INDEX_HTML);

    HTTP_200;
    if (file_exists(index_html)) {
      read_file(index_html);
    } else {
      printf("Hello! You are using %s\n\n", request_header("User-Agent"));
    }
  }

  GET("/test") {
    HTTP_200;
    printf("List of request headers:\n\n");

    header_t *h = request_headers();

    while (h->name) {
      printf("%s: %s\n", h->name, h->value);
      h++;
    }
  }

  POST("/") {
    HTTP_201;
    printf("Wow, seems that you POSTed %d bytes.\n", payload_size);
    printf("Fetch the data using `payload` variable.\n");
    if (payload_size > 0)
      printf("Request body: %s", payload);
  }

  GET(uri) {
    char file_name[255];
    sprintf(file_name, "%s%s", PUBLIC_DIR, uri);

    if (file_exists(file_name)) {
      HTTP_200;
      read_file(file_name);
    } else {
      HTTP_404;
      sprintf(file_name, "%s%s", PUBLIC_DIR, NOT_FOUND_HTML);
      if (file_exists(file_name))
        read_file(file_name);
    }
  }

  ROUTE_END()
}
