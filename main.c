#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);     // port
  addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0
  int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  struct sockaddr_in {
    uint16_t sin_family;     // AF_INET
    uint16_t sin_port;       // port in big-endian
    struct in_addr sin_addr; // IPv4
  };
  struct in_addr {
    uint32_t s_addr; // IPv4 in big-endian
  };

  // uint32_t htonl(uint32_t hostlong);  CPU endian <=> big-endian
  // uint16_t htons(uint16_t hostshort); same; 16-bit
  // uint32_t htonl(uint32_t hostlong);
  // uint32_t ntohl(uint32_t netlong); n-to-h is the same as h-to-a
  // uint32_t htobe32(uint32_t host_32bits); "be" stands for big-endian
  // uint32_t be32toh(uint32_t big_endian_32bits);

  int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
}
