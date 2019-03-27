# Pico HTTP Server in C

This is a very simple HTTP server for Unix, using `fork()`. It's very easy to use.

## How to use

1. Include header `httpd.h`.
2. Write your route method, handling requests.
3. Call `serve_forever("8000")` to start serving on http://127.0.0.1:8000/.

See `main.c`, an interesting example.

To log stuff, use `fprintf(stderr, "message");`

View `httpd.h` for more information.

## Test example

Open http://localhost:8000/test in browser to see request headers.

## Links

Reworked and refactored from <https://gist.github.com/laobubu/d6d0e9beb934b60b2e552c2d03e1409e>.

Based on <http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html>.
