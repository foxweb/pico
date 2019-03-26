#include "httpd.h"

int main(int c, char **v) {
  serve_forever("8000");
  return 0;
}

void route() {
  ROUTE_START()

  ROUTE_GET("/") {
    printf("HTTP/1.1 200 OK\n\n");
    printf("Hello! You are using %s", request_header("User-Agent"));
  }

  ROUTE_POST("/") {
    printf("HTTP/1.1 200 OK\n\n");
    printf("Wow, seems that you POSTed %d bytes. \n", payload_size);
    printf("Fetch the data using `payload` variable.");
  }

  ROUTE_END()
}
