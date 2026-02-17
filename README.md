# Pico HTTP Server in C

A minimal HTTP server for Unix/Linux systems using fork-based concurrency. Simple, lightweight, and easy to understand.

## ⚠️ Security Warning

**This server is designed for development and learning purposes only.** It should NOT be used in production environments without thorough security review and hardening. While recent updates have addressed many security issues, it is still a minimal implementation without comprehensive protection.

## System Requirements

- **Operating System**: POSIX-compliant Unix/Linux (macOS, Linux, BSD)
- **Compiler**: GCC or Clang with C99 support
- **Build Tool**: GNU Make
- **Dependencies**: Standard C library, POSIX headers

## Features

- Simple routing with `GET()`, `POST()`, and `HEAD()` macros
- Static file serving from `./public` directory
- **Directory listing** - nginx-like automatic index with file sizes and dates
- **Auto index.html** - Automatic detection in subdirectories
- Request header parsing and access
- POST payload handling
- HEAD method support for checking resource existence
- Fork-based concurrency (up to 1000 concurrent connections)
- Path traversal protection
- Graceful shutdown with signal handling

## Security Features

✅ Path traversal attack prevention  
✅ Buffer overflow protection  
✅ Format string vulnerability fixes  
✅ NULL pointer dereference checks  
✅ Input validation for HTTP requests  

## Quick Start

### Build

```sh
make
```

### Run

```sh
./server          # Default port 8000
./server 3000     # Custom port
```

### Test

Open in your browser:
- http://localhost:8000/ - Index page or default greeting
- http://localhost:8000/test - View system info & request headers
- http://localhost:8000/[any-file] - Serve static files from `./public`

### Stop

Press `Ctrl+C` for graceful shutdown.

## How to Use

### 1. Basic Usage

```c
#include "httpd.h"

int main() {
    serve_forever("8000");
    return 0;
}
```

### 2. Define Routes

```c
void route() {
    ROUTE_START()

    GET("/hello") {
        HTTP_200;
        printf("Hello, World!\n");
    }

    GET("/test") {
        HTTP_200;
        // Display system information
        printf("Server Uptime: ...\n");
        printf("OS: %s\n", ...);
        // Full implementation in main.c
    }

    POST("/data") {
        HTTP_201;
        printf("Received %d bytes\n", payload_size);
        // Access POST data via: payload, payload_size
    }

    HEAD("/file.pdf") {
        // Check if resource exists without sending body
        if (file_exists("public/file.pdf")) {
            HTTP_200;
            // Headers sent, no body (HEAD behavior)
        } else {
            HTTP_404;
        }
    }

    GET(uri) {
        // Catch-all route for static files
        serve_static_file(uri);
    }

    ROUTE_END()
}
```

### 3. Access Request Data

```c
// Get specific header
char *user_agent = request_header("User-Agent");

// Get all headers
header_t *headers = request_headers();
while (headers->name) {
    printf("%s: %s\n", headers->name, headers->value);
    headers++;
}

// Access request components
printf("Method: %s\n", method);  // "GET" or "POST"
printf("URI: %s\n", uri);        // "/path/to/resource"
printf("Query: %s\n", qs);       // "key=value&foo=bar"
printf("Protocol: %s\n", prot);  // "HTTP/1.1"
```

## Project Structure

```
pico/
├── main.c          # Entry point and example routes
├── httpd.c         # HTTP server implementation
├── httpd.h         # API header and macros
├── Makefile        # Build configuration
├── README.md       # Documentation
└── public/         # Static files directory
    ├── index.html
    ├── 404.html
    └── ...
```

## Configuration

- **Port**: Pass as command-line argument (default: 8000)
- **Public Directory**: `./public` (defined in `main.c`)
- **Max Connections**: 1000 (defined in `httpd.c`)
- **Buffer Size**: 65535 bytes (defined in `httpd.c`)

## Limitations and Known Issues

1. **Fork-based concurrency**: Creates a new process for each connection, which is inefficient for high-traffic scenarios
2. **No HTTPS support**: Plain HTTP only
3. **No HTTP/2 or HTTP/3**: HTTP/1.1 only
4. **Limited HTTP compliance**: Minimal implementation, not fully HTTP/1.1 compliant
5. **No authentication**: All endpoints are publicly accessible
6. **No compression**: No gzip or other compression support
7. **File size limits**: Large file uploads may hit buffer limits

## Testing and Benchmarking

Use [Siege](https://github.com/JoeDog/siege) for load testing:

```sh
siege -i -f urls.txt
siege -c 100 -r 10 http://localhost:8000/
```

## Logging

Use `fprintf(stderr, "message")` for logging. All logs are sent to stderr.

```c
fprintf(stderr, "Processing request: %s %s\n", method, uri);
```

## Building Custom Applications

See `main.c` for a complete example. Key steps:

1. Include `httpd.h`
2. Implement `route()` function with your endpoints
3. Call `serve_forever(port)` in `main()`
4. Compile with: `gcc -o myserver main.c httpd.c`

## API Reference

### Server Control

- `void serve_forever(const char *port)` - Start server on specified port

### Response Macros

- `HTTP_200` - Send 200 OK response
- `HTTP_201` - Send 201 Created response
- `HTTP_404` - Send 404 Not Found response
- `HTTP_500` - Send 500 Internal Server Error response

### Routing Macros

- `ROUTE_START()` - Begin route definition
- `GET(path)` - Define GET route
- `POST(path)` - Define POST route
- `HEAD(path)` - Define HEAD route (returns headers only, no body)
- `ROUTE_END()` - End route definition

### Request Functions

- `char *request_header(const char *name)` - Get specific header value
- `header_t *request_headers(void)` - Get all headers array

### Global Variables

- `char *method` - HTTP method ("GET", "POST")
- `char *uri` - Request URI
- `char *qs` - Query string
- `char *prot` - HTTP protocol version
- `char *payload` - POST request body
- `int payload_size` - POST body size in bytes

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Security

Report security issues via the repository issues page. See [SECURITY.md](SECURITY.md) for details.

## License

MIT License - see LICENSE file for details.

## Links

Reworked and refactored from <https://gist.github.com/laobubu/d6d0e9beb934b60b2e552c2d03e1409e>.

Based on <http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html>.
