#include "httpd_priv.h"


static header_t reqhdr[17] = {{"\0", "\0"}};

// Client request
char *methodstr,  // "GET" or "POST"
     *uri,         // "/index.html" things before '?'
     *querystr,    // "a=1&b=2" things after  '?'
     *prot,        // "HTTP/1.1"
     *payload;     // for POST

int payload_size;
int method;

// get request header by name
char *request_header(const char *name)
{
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
static void uri_unescape(char *uri)
{
  char  chr = 0;
  char *src = uri;
  char *dst = uri;
  char  hex[4] = {0};
  
  assert(hex[3]=='\0');
  
  // Skip inital non encoded character
  while (*src && !isspace((int)(*src)) && (*src != '%'))
    src++;

  // Replace encoded characters with corresponding code.
  dst = src;
  while (*src && !isspace((int)(*src))) {
    chr = *src++;
    if (chr == '%') {
      hex[0] = src[0]; hex[1] = src[1];
      if ((chr = strtol(hex,NULL,16)) == 0) 
        chr = '%';
      else 
        src += 2;
    } 
    else if (chr == '+')
      chr = ' ';

   *dst++ = chr;
  }
  *dst = '\0';
}

static int method_code(char *meth)
{
  int code;

  switch (*meth) {
    case 'P' : code = (meth[1] == 'O') ? METHOD_POST : METHOD_PUT;
               break;

    case 'G' : code = METHOD_GET;      break;
    case 'H' : code = METHOD_HEAD;     break;
    case 'D' : code = METHOD_DELETE;   break;
    case 'O' : code = METHOD_OPTIONS;  break;
    case 'T' : code = METHOD_TRACE;    break;
    default  : code = METHOD_NONE;     break;
  }

  return code;
}


/*
                     _________________              __________
                    V                 \            /          V
    -->[REQ LINE] --o-> [HEADER LINE] -o-->[CRLF]-o-->[BODY]--o-->[end]
*/

char linebuf[1024];

int get_request(int clientfd, char *buffer)
{
  int ret = 0; 
  int rcvd;

  rcvd = recv(clientfd, buffer, BUF_SIZE, 0);
  
  if (rcvd < 0) // receive error
    fprintf(stderr, ("ERROR: recv() error\n"));
  else if (rcvd == 0) // receive socket closed
    fprintf(stderr, "INFO: Client disconnected.\n");
  else { // message received
    buffer[rcvd] = '\0';

    methodstr = strtok(buffer, " \t\r\n");
    method = method_code(methodstr);

    if (method != METHOD_NONE) {
      uri = strtok(NULL, " \t");
      querystr = strchr(uri, '?');

      prot = strtok(NULL, " \t\r\n");
  
      uri_unescape(uri);
      fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", METHOD_STR(method), uri);
  
      if (querystr)
        *querystr++ = '\0'; // split URI
      else
        querystr = uri - 1; // use an empty string
  
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
      payload_size = t2 ? atol(t2) : (rcvd - (t - buffer));
      ret = 1;
    }
  }

  return ret;
}
