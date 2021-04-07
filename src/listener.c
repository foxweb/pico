#include "httpd_priv.h"

static int listenfd;

static void start_server(const char *);

// client connection
static void respond(int clientfd)
{
  int goodrequest = 0;
  char *buffer = NULL;

  buffer = malloc(BUF_SIZE);
  goodrequest = get_request(clientfd,buffer);

  // bind clientfd to stdout, making it easier to write
  dup2(clientfd, STDOUT_FILENO);
 
  if (goodrequest)
    httpd_route_();  // call router
  else
    HTTP_400;

  // tidy up
  fflush(stdout);
  shutdown(STDOUT_FILENO, SHUT_WR);
  close(STDOUT_FILENO);

  free(buffer);
}

// start server
static void start_server(const char *port)
{
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
  if (listen(listenfd, SOMAXCONN ) != 0) {
    perror("listen() error");
    exit(1);
  }
}

void httpd_start(const char *PORT)
{
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  int clientfd;

  fprintf(stderr, "Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", PORT,
         "\033[0m");

  start_server(PORT);

  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);

  // ACCEPT connections
  addrlen = sizeof(clientaddr);
  while (1) {
    clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

    if (clientfd < 0) {
      perror("accept() error");
      exit(1);
    } else {
      if (fork() == 0) { // child
        close(listenfd);
        respond(clientfd);
        close(clientfd);
        exit(0);
      } else { // parent
        close(clientfd);
      }
    }
  }
}
