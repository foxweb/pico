/**
 * @file httpd.h
 * @brief Simple HTTP server API for Unix/Linux systems
 *
 * This header provides a minimal HTTP server implementation with fork-based
 * concurrency. Includes routing macros and request/response utilities.
 */

#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <stdio.h>
#include <string.h>

/** @brief HTTP request method (e.g., "GET", "POST") */
extern char *method;

/** @brief Request URI path before query string (e.g., "/index.html") */
extern char *uri;

/** @brief Query string after '?' (e.g., "a=1&b=2") */
extern char *qs;

/** @brief HTTP protocol version (e.g., "HTTP/1.1") */
extern char *prot;

/** @brief POST request payload data */
extern char *payload;

/** @brief Size of POST payload in bytes */
extern int payload_size;

/**
 * @brief Start the HTTP server and listen for connections
 *
 * This function initializes the server, binds to the specified port,
 * and enters an infinite loop accepting and handling client connections.
 * Each connection is handled in a separate forked process.
 *
 * @param PORT Port number as a string (e.g., "8000")
 * @note This function never returns unless there's a fatal error
 * @warning Blocks indefinitely - use Ctrl+C or signals to stop
 */
void serve_forever(const char *PORT);

/**
 * @brief Get the value of a specific HTTP request header
 *
 * @param name Header name (case-sensitive, e.g., "User-Agent")
 * @return Pointer to header value string, or NULL if not found
 * @note The returned pointer is valid only for the current request
 */
char *request_header(const char *name);

/**
 * @brief Structure representing an HTTP header name-value pair
 */
typedef struct {
  char *name;   /**< Header name */
  char *value;  /**< Header value */
} header_t;

/** @brief Array storing parsed request headers (max 17 headers) */
extern header_t reqhdr[17];

/**
 * @brief Get all HTTP request headers as an array
 *
 * @return Pointer to array of header_t structures
 * @note Array is terminated with a header where name is NULL
 * @note Valid only for the current request
 */
header_t *request_headers(void);

/**
 * @brief User-defined routing function
 *
 * This function must be implemented by the user to define request handling.
 * Use ROUTE_START(), GET(), POST(), and ROUTE_END() macros to define routes.
 *
 * @note Called once per request in the forked child process
 * @see ROUTE_START, GET, POST, ROUTE_END
 */
void route();

/* ==================== Response Macros ==================== */

/** @brief HTTP protocol version string */
#define RESPONSE_PROTOCOL "HTTP/1.1"

/** @brief Send HTTP 200 OK response header */
#define HTTP_200 printf("%s 200 OK\n\n", RESPONSE_PROTOCOL)

/** @brief Send HTTP 201 Created response header */
#define HTTP_201 printf("%s 201 Created\n\n", RESPONSE_PROTOCOL)

/** @brief Send HTTP 404 Not Found response header */
#define HTTP_404 printf("%s 404 Not found\n\n", RESPONSE_PROTOCOL)

/** @brief Send HTTP 500 Internal Server Error response header */
#define HTTP_500 printf("%s 500 Internal Server Error\n\n", RESPONSE_PROTOCOL)

/* ==================== Routing Macros ==================== */

/**
 * @brief Begin route definitions
 *
 * Must be the first statement in the route() function.
 * Use with GET(), POST(), and ROUTE_END() macros.
 *
 * @code
 * void route() {
 *     ROUTE_START()
 *     GET("/hello") {
 *         HTTP_200;
 *         printf("Hello World");
 *     }
 *     ROUTE_END()
 * }
 * @endcode
 */
#define ROUTE_START() if (0) {

/**
 * @brief Define a route for a specific HTTP method and URI
 *
 * @param METHOD HTTP method string (e.g., "GET", "POST")
 * @param URI URI path to match (e.g., "/hello")
 */
#define ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if (strcmp(URI, uri) == 0 && strcmp(METHOD, method) == 0) {

/**
 * @brief Define a GET route
 *
 * @param URI URI path to match (e.g., "/users")
 *
 * @code
 * GET("/users") {
 *     HTTP_200;
 *     printf("User list");
 * }
 * @endcode
 */
#define GET(URI) ROUTE("GET", URI)

/**
 * @brief Define a POST route
 *
 * @param URI URI path to match (e.g., "/users")
 *
 * @code
 * POST("/users") {
 *     HTTP_201;
 *     printf("User created: %s", payload);
 * }
 * @endcode
 */
#define POST(URI) ROUTE("POST", URI)

/**
 * @brief Define a HEAD route
 *
 * HEAD method is identical to GET except the server does not return
 * the message body in the response. Useful for checking if a resource
 * exists or getting metadata without downloading the full content.
 *
 * @param URI URI path to match (e.g., "/file.txt")
 *
 * @code
 * HEAD("/file.txt") {
 *     if (file_exists("public/file.txt")) {
 *         HTTP_200;
 *         // Headers sent, no body
 *     } else {
 *         HTTP_404;
 *     }
 * }
 * @endcode
 */
#define HEAD(URI) ROUTE("HEAD", URI)

/**
 * @brief End route definitions
 *
 * Must be the last statement in the route() function.
 * If no routes match, sends HTTP 500 error.
 */
#define ROUTE_END()                                                            \
  }                                                                            \
  else HTTP_500;

#endif
