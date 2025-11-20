#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  fprintf(stderr, "Client says: %s\n", rbuf);

  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  // this is needed for most server applications
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // bind
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
  int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // listen
  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen");
  }

  while (1) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0) {
      continue; // error
    }
    // do_something(connfd);

    // only serves one client connection at once;
    while (1) {
      int32_t err = one_request(connfd);
      if (err) {
        break;
      }
      /* The one_request function will read 1 request and write 1 response. The
       * problem is, how does it know how many bytes to read? This is the
       * primary function of an application protocol. Usually protocol has 2
       * levels of structures:
       * 1. A high-level structure to split the byte stream into messages.
       * 2. The structure within a mess;age, a.k.a. deserialization.
       *
       * A simple binary protocol
       * We will do the first step is to split the byte stream into messages.
       * For now, both the request and response messages are just strings. len
       * msg1  len msg2  more.. 4B   ...   4B  ... Each message consists of a
       * 4-byte little-endian integer indicating the length of the request and
       * the variable-length payload. This is not the real Redic protocol. We'll
       * discuss the alternative protocol designs later.*/
    }
    close(connfd);
  }
  return 0;
}
