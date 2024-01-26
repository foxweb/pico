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

// Use (2^n-1) to speedup modulo
#define MAX_CONNECTIONS 1023
#define BUF_SIZE 65535

static void start_server(const char *);
static void respond(int);

static int listenfd;
int *clients;
static char *buf;

static header_t reqhdr[17] = {{"\0", "\0"}};

// Client request
char *method_str, // "GET" or "POST"
    *uri,         // "/index.html" things before '?'
    *qs,          // "a=1&b=2" things after  '?'
    *prot,        // "HTTP/1.1"
    *payload;     // for POST

int payload_size;
int method;

int nxt_slot(int slot) {
  int nxt_slot = slot;

  do {
    nxt_slot = (nxt_slot + 1) & MAX_CONNECTIONS;

    if (nxt_slot == slot) { // There is no slot available for a new client!
      fprintf(stderr, "WARNING: no available connection\n");
      usleep(250); // Let's wait some millisecond
    }
  } while (clients[nxt_slot] != -1);

  return nxt_slot;
}

void serve_forever(const char *PORT) {
  struct sockaddr_in clientaddr;
  socklen_t addrlen;

  int slot = 0;

  printf("Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
         "\033[0m");

  // create shared memory for client slot array
  clients = mmap(NULL, sizeof(*clients) * (MAX_CONNECTIONS + 1),
                 PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  // Setting all elements to -1: signifies there is no client connected
  for (int i = 0; i <= MAX_CONNECTIONS; i++)
    clients[i] = -1;

  start_server(PORT);

  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);

  // ACCEPT connections
  addrlen = sizeof(clientaddr);
  while (1) {
    clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

    if (clients[slot] < 0) {
      perror("accept() error");
      exit(1);
    } else {
      if (fork() == 0) {
        close(listenfd);
        respond(slot);
        close(clients[slot]);
        clients[slot] = -1;
        exit(0);
      } else {
        close(clients[slot]);
      }
    }
    // Look for slot to be used for next client
    slot = nxt_slot(slot);
  }
}

// start server
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

// get request header by name
char *request_header(const char *name) {
  header_t *h = reqhdr;
  while (h->name) {
    if (strcmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

// get all request headers
header_t *request_headers(void) { return reqhdr; }

// Handle escape characters (%xx)
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
    else if ((*src == '%') && isxdigit((int)(src[1])) &&
             isxdigit((int)(src[2]))) {
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

static int method_code(char *meth) {
  int code;

  switch (*meth) {
  case 'P':
    code = (meth[1] == 'O') ? METHOD_POST : METHOD_PUT;
    break;

  case 'G':
    code = METHOD_GET;
    break;
  case 'H':
    code = METHOD_HEAD;
    break;
  case 'D':
    code = METHOD_DELETE;
    break;
  case 'O':
    code = METHOD_OPTIONS;
    break;
  case 'T':
    code = METHOD_TRACE;
    break;
  default:
    code = METHOD_NONE;
    break;
  }

  return code;
}

// client connection
void respond(int slot) {
  int rcvd;

  buf = malloc(BUF_SIZE);
  rcvd = recv(clients[slot], buf, BUF_SIZE, 0);

  if (rcvd < 0) // receive error
    fprintf(stderr, ("ERROR: recv() error\n"));
  else if (rcvd == 0) // receive socket closed
    fprintf(stderr, "INFO: Client disconnected.\n");
  else // message received
  {
    buf[rcvd] = '\0';

    method_str = strtok(buf, " \t\r\n");
    method = method_code(method_str);

    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    uri_unescape(uri);

    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", METHOD_STR(method), uri);

    qs = strchr(uri, '?');

    if (qs)
      *qs++ = '\0'; // split URI
    else
      qs = uri - 1; // use an empty string

    header_t *h = reqhdr;
    char *t, *t2;
    while (h < reqhdr + 16) {
      char *key, *val;

      key = strtok(NULL, "\r\n: \t");
      if (!key)
        break;

      val = strtok(NULL, "\r\n");
      while (*val && *val == ' ')
        val++;

      h->name = key;
      h->value = val;
      h++;
      fprintf(stderr, "[H] %s: %s\n", key, val);
      t = val + 1 + strlen(val);
      if (t[1] == '\r' && t[2] == '\n')
        break;
    }
    t = strtok(NULL, "\r\n");
    t2 = request_header("Content-Length"); // and the related header if there is
    payload = t;
    payload_size = t2 ? atol(t2) : (rcvd - (t - buf));

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
