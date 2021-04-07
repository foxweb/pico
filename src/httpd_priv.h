
#ifndef HTTPD_PRIV_H___
#define HTTPD_PRIV_H___

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

#include "httpd.h"

#define BUF_SIZE 65536

int get_request(int clientfd, char *buffer);


//#include "utl.h"
#endif
