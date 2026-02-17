#include "httpd.h"
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR "./public"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

int main(int c, char **v) {
  char *port = c == 1 ? "8000" : v[1];
  serve_forever(port);
  return 0;
}

// Validate path to prevent directory traversal attacks
int is_path_safe(const char *path, const char *base_dir) {
  char real_path[PATH_MAX];
  char real_base[PATH_MAX];

  // Get canonical paths
  if (realpath(base_dir, real_base) == NULL) {
    return 0;
  }

  // For non-existent files, check the directory part
  char temp_path[PATH_MAX];
  snprintf(temp_path, sizeof(temp_path), "%s", path);

  // Check if path exists, if not try to resolve its parent
  if (realpath(temp_path, real_path) == NULL) {
    // Path doesn't exist, check if it would be inside base_dir
    // by checking if the path starts with ../ or is absolute
    if (path[0] == '/' || strstr(path, "../") != NULL) {
      return 0;
    }
    // Additional check: construct the full path and verify it starts with base
    snprintf(real_path, sizeof(real_path), "%s", temp_path);
  }

  // Verify the resolved path is within base directory
  size_t base_len = strlen(real_base);
  if (strncmp(real_path, real_base, base_len) != 0) {
    return 0;
  }

  // Ensure the path is either exactly the base or starts with base/
  if (real_path[base_len] != '\0' && real_path[base_len] != '/') {
    return 0;
  }

  return 1;
}

int file_exists(const char *file_name) {
  struct stat buffer;
  int exists;

  exists = (stat(file_name, &buffer) == 0);

  return exists;
}

int read_file(const char *file_name) {
  char buf[CHUNK_SIZE];
  FILE *file;
  size_t nread;
  int err = 1;

  file = fopen(file_name, "r");

  if (file) {
    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
      fwrite(buf, 1, nread, stdout);

    err = ferror(file);
    fclose(file);
  }
  return err;
}

// Build path in public directory with safety check
int build_public_path(char *dest, size_t dest_size, const char *relative_path) {
  int result = snprintf(dest, dest_size, "%s%s", PUBLIC_DIR, relative_path);
  if (result < 0 || result >= dest_size) {
    return -1; // Path too long
  }
  return 0;
}

// Serve static file from public directory
void serve_static_file(const char *relative_path) {
  char file_path[256];

  if (build_public_path(file_path, sizeof(file_path), relative_path) < 0) {
    HTTP_500;
    printf("Internal error\n");
    return;
  }

  // Validate path to prevent directory traversal
  if (!is_path_safe(file_path, PUBLIC_DIR)) {
    HTTP_404;
    printf("Access denied\n");
    return;
  }

  if (file_exists(file_path)) {
    HTTP_200;
    read_file(file_path);
  } else {
    HTTP_404;
    // Try to serve 404 page
    if (build_public_path(file_path, sizeof(file_path), NOT_FOUND_HTML) == 0 &&
        file_exists(file_path)) {
      read_file(file_path);
    } else {
      printf("File not found\n");
    }
  }
}

void route() {
  ROUTE_START()

  GET("/") {
    char index_html[256];
    if (build_public_path(index_html, sizeof(index_html), INDEX_HTML) == 0 &&
        file_exists(index_html)) {
      HTTP_200;
      read_file(index_html);
    } else {
      HTTP_200;
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
    if (payload_size > 0) {
      printf("Request body: ");
      // Use fputs to avoid format string vulnerabilities
      fwrite(payload, 1, payload_size, stdout);
    }
  }

  GET(uri) {
    serve_static_file(uri);
  }

  ROUTE_END()
}
