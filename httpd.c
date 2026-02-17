#include "httpd.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define MAX_CONNECTIONS 1000
#define BUF_SIZE 65535

static int listenfd;
int *clients;
static void start_server(const char *);
static void respond(int);
static void cleanup_and_exit(int signum);

static char *buf;

// Request headers storage
header_t reqhdr[17] = {{"\0", "\0"}};

// Server start time for uptime calculation
time_t server_start_time;

// Client request
char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2" things after  '?'
    *prot,    // "HTTP/1.1"
    *payload; // for POST

int payload_size;

/**
 * @brief Cleanup function for graceful server shutdown
 *
 * Called when SIGINT (Ctrl+C) or SIGTERM signals are received.
 * Closes the listening socket and frees shared memory before exiting.
 *
 * @param signum Signal number that triggered the handler
 */
void cleanup_and_exit(int signum) {
  fprintf(stderr, "\nShutting down server...\n");

  // Close listening socket
  if (listenfd >= 0) {
    close(listenfd);
  }

  // Free shared memory
  if (clients != NULL) {
    munmap(clients, sizeof(*clients) * MAX_CONNECTIONS);
  }

  exit(0);
}

void serve_forever(const char *PORT) {
  struct sockaddr_in clientaddr;
  socklen_t addrlen;

  int slot = 0;

  // Record server start time for uptime calculation
  server_start_time = time(NULL);

  fprintf(stderr, "Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
         "\033[0m");

  // create shared memory for client slot array
  clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS,
                 PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  if (clients == MAP_FAILED) {
    perror("mmap() error");
    exit(1);
  }

  // Setting all elements to -1: signifies there is no client connected
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++)
    clients[i] = -1;
  start_server(PORT);

  // Setup signal handlers for graceful shutdown using sigaction
  struct sigaction sa_exit;
  sa_exit.sa_handler = cleanup_and_exit;
  sigemptyset(&sa_exit.sa_mask);
  sa_exit.sa_flags = 0;
  sigaction(SIGINT, &sa_exit, NULL);
  sigaction(SIGTERM, &sa_exit, NULL);

  // Ignore SIGCHLD to avoid zombie processes
  struct sigaction sa_chld;
  sa_chld.sa_handler = SIG_IGN;
  sigemptyset(&sa_chld.sa_mask);
  sa_chld.sa_flags = SA_NOCLDWAIT; // Automatically reap child processes
  sigaction(SIGCHLD, &sa_chld, NULL);

  // ACCEPT connections
  while (1) {
    addrlen = sizeof(clientaddr);
    clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

    if (clients[slot] < 0) {
      perror("accept() error");
      exit(1);
    } else {
      if (fork() == 0) {
        // Child process: close listening socket and handle request
        close(listenfd);
        respond(slot);
        close(clients[slot]);
        clients[slot] = -1;
        exit(0);
      }
      // Parent process: don't close the client socket, let child handle it
      // The child will set clients[slot] = -1 when done
    }

    // Find next available slot
    while (clients[slot] != -1)
      slot = (slot + 1) % MAX_CONNECTIONS;
  }
}

/**
 * @brief Initialize and start the HTTP server
 *
 * Creates a socket, binds to the specified port, and starts listening
 * for incoming connections. Does not accept connections - that's done
 * by serve_forever().
 *
 * @param port Port number as a string (e.g., "8000")
 */
void start_server(const char *port) {
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    perror("getaddrinfo() error");
    exit(1);
  }
  // socket and bind
  for (p = res; p != NULL; p = p->ai_next) {
    int option = 1;
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (listenfd == -1)
      continue;
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
  }
  if (p == NULL) {
    perror("socket() or bind()");
    exit(1);
  }

  freeaddrinfo(res);

  // listen for incoming connections
  if (listen(listenfd, SOMAXCONN) != 0) {
    perror("listen() error");
    exit(1);
  }
}

/**
 * @brief Get the value of a specific HTTP request header
 *
 * Searches the request headers array for a header with the given name.
 * Comparison is case-sensitive.
 *
 * @param name Header name to search for (e.g., "User-Agent")
 * @return Pointer to header value string, or NULL if not found
 */
char *request_header(const char *name) {
  header_t *h = reqhdr;
  while (h->name) {
    if (strcmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

/**
 * @brief Get all HTTP request headers
 *
 * Returns a pointer to the array of parsed request headers.
 * The array is terminated with an entry where name is NULL.
 *
 * @return Pointer to array of header_t structures
 */
header_t *request_headers(void) { return reqhdr; }

/**
 * @brief Decode URL-encoded characters in URI
 *
 * Converts percent-encoded characters (%XX) and plus signs (+)
 * to their actual character values. Modifies the string in-place.
 *
 * @param uri URI string to decode (modified in-place)
 */
static void uri_unescape(char *uri) {
  char chr = 0;
  char *src = uri;
  char *dst = uri;

  // Skip inital non encoded character
  while (*src && !isspace((int)(*src)) && (*src != '%'))
    src++;

  // Replace encoded characters with corresponding code.
  dst = src;
  while (*src && !isspace((int)(*src))) {
    if (*src == '+')
      chr = ' ';
    else if ((*src == '%') && src[1] && src[2]) {
      src++;
      chr = ((*src & 0x0F) + 9 * (*src > '9')) * 16;
      src++;
      chr += ((*src & 0x0F) + 9 * (*src > '9'));
    } else
      chr = *src;
    *dst++ = chr;
    src++;
  }
  *dst = '\0';
}

/**
 * @brief Handle a client connection and process the HTTP request
 *
 * Receives and parses the HTTP request, extracts method, URI, headers,
 * and payload. Then calls the user-defined route() function to handle
 * the request. Runs in a forked child process.
 *
 * @param slot Index in the clients array for this connection
 */
void respond(int slot) {
  int rcvd;

  buf = malloc(BUF_SIZE);
  if (buf == NULL) {
    fprintf(stderr, "malloc() error\n");
    return;
  }

  rcvd = recv(clients[slot], buf, BUF_SIZE, 0);

  if (rcvd < 0) { // receive error
    fprintf(stderr, ("recv() error\n"));
    free(buf);
    return;
  } else if (rcvd == 0) { // receive socket closed
    fprintf(stderr, "Client disconnected unexpectedly.\n");
    free(buf);
    return;
  }
  else // message received
  {
    // Prevent buffer overflow: ensure we don't write past the buffer
    if (rcvd >= BUF_SIZE)
      rcvd = BUF_SIZE - 1;
    buf[rcvd] = '\0';

    method = strtok(buf, " \t\r\n");
    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    // Validate parsed values
    if (!method || !uri || !prot) {
      fprintf(stderr, "Invalid HTTP request\n");
      free(buf);
      return;
    }

    uri_unescape(uri);

    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);

    qs = strchr(uri, '?');

    if (qs)
      *qs++ = '\0'; // split URI
    else
      qs = ""; // use an empty string

    header_t *h = reqhdr;
    char *t, *t2;
    while (h < reqhdr + 16) {
      char *key, *val;

      key = strtok(NULL, "\r\n: \t");
      if (!key)
        break;

      val = strtok(NULL, "\r\n");
      if (!val)
        break;

      while (*val && *val == ' ')
        val++;

      h->name = key;
      h->value = val;
      h++;
      fprintf(stderr, "[H] %s: %s\n", key, val);
      t = val + 1 + strlen(val);
      if (t >= buf + rcvd - 2) // Bounds check
        break;
      if (t[1] == '\r' && t[2] == '\n')
        break;
    }
    t = strtok(NULL, "\r\n");
    t2 = request_header("Content-Length"); // and the related header if there is
    payload = t;
    payload_size = 0;
    if (t2) {
      payload_size = atol(t2);
    } else if (t && t >= buf && t < buf + rcvd) {
      payload_size = rcvd - (t - buf);
    }

    // bind clientfd to stdout, making it easier to write
    int clientfd = clients[slot];
    dup2(clientfd, STDOUT_FILENO);
    close(clientfd);

    // call router
    route();

    // tidy up
    fflush(stdout);
    shutdown(STDOUT_FILENO, SHUT_WR);
    close(STDOUT_FILENO);
  }

  free(buf);
}
