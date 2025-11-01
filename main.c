#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>



// tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
// udp_socket = socket(AF_INET, SOCK_DGRAM, 0);



// Step 1: Obtain a socket handle
// The socket() syscall takes 3 integer arguments.
int fd = socket(AF_INET, SOCK_STREAM, 0);

/*
1. AF_INET is for IPv4. Use AF_INET6 for IPv6 or dual-stack sockets.
2. SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP.
3. The 3rd argument is 0 and useless for our purposes.
The combination of the 3 arguments determines the socket protocol:
Protocol           Arguments
IPv4+TCP           socket(AF_INET, SOCK_STREAM, 0)
IPv6+TCP           socket(AF_INET6, SOCK_STREAM, 0)
IPv4+UDP           socket(AF_INET, SOCK_DGRAM, 0)
IPv6+UDP           socket(AF_INET6, SOCK_DGRAM, 0)
*/

// man socket.2 lists all the flags, but only certain combinations are accepted. We'll only be using TCP, so you can forget about those arguments for now. By the way, man ip.7 tells you how to create TCP/UDP sockets and the required #includes.

// Step 2: Set socket options
/* There are many options that change the behavior of a socket, such as TCP no delay, IP QoS, etc. (none are our concern). These options are set via the setsockopt() API. Like the bind() API, this just passs a parameter to the OS as the actual socket has not been created yet.
 */
int val = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
/* The combination of the 2nd & 3rd arguments specifies which option to set.
The 4th argument is the option value.
Different options use different types, so the size of the option value is also needed.
In this case, we set the SO_REUSEADDR option to an int value of 1, this option accepts a boolean value of 0 or 1. What does this do? This is related to delayed packets and TCP TIME_WAIT. Understanding this requires a non-trivial amount of TCP knowledge, you can read the explanations.
https://stackoverflow.com/a/3233022
The effect of SO_REUSEADDR is important: if it's not set to 1, a server program cannot bind to the same IP:port it was using after a restart. This is generally undesirable TCP behavior. You should enable SO_REUSEADDR for all listening sockets! Even if you don't understand what exactly it is.
You can look up other socket options in man socket.7, man ip.7, man tcp.7, but don't expect to understand them all.
*/


// Step 3: Bind to an address

// We'll bind to the wildcard address 0.0.0.0:1234. This is just a parameter for listen()
struct sockaddr_in addr = {};
addr.sin_family = AF_INET;
addr.sin_port = htons(1234); // port
addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0
int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
if (rv) { die("bind()"); }

/* struct sockaddr_in holds an IPv4:port pair stored as big-endian numbers, converted by htoms() and htonl(). For example. 1.2.3.4 is represents by htonl(0x01020304).
 */
struct sockaddr_in {
  uint16_t sin_family; // AF_INET
  uint16_t sin_port; // port in big-endian
  struct in_addr sin_addr; // IPv4
};
struct in_addr {
  uint32_t s_addr; // IPv4 in big-endian
};

/* SIDE NOTE: ENDIAN-NESS
   There are 2 ways to store integers in memory.
  * Little-endian: The least significant byte comes first.
  * Big-endian (called network byte order): The most significant byte commes first.

  *****************************************************************
  Order           uint16_t(0x0304)            uint32_t(0x01020304)
  *****************************************************************
  LE              04 03                       04 03 02 01
  BE              03 04                       01 02 03 04
  *****************************************************************

  The difference is the order of bytes. Reversing the byte order is called "byte swap".

  In the past, there are CPUs of both endians, a protocol or a data format must choose one of the order, the software should convert between the CPU endian and the format endian. In 2025, only little-endian CPUs are relevant. If a format uses big-endian, a byte swap is needed.
*/ 

uint32_t htonl(uint32_t hostlong); // CPU endian <=> big-endian
uint16_t htons(uint16_t hostshort); // same; 16-bit

/* htonl() reads "Host to Network Long". "Host" means the CPU endian. "Network" means big-endian. "Long" actually means uint32_t, not the long type. On little-endian CPUs, it's a byte swap. On big-endian CPUs, it does nothing. The function name is funny, what's funnier is that there are 4 names for the same function.
*/

uint32_t htonl(uint32_t hostlong);
uint32_t ntohl(uint32_t netlong); // n-to-h is the same as h-to-n
uint32_t htobe32(uint32_t host_32bits); //  "be" stands for big-endian
uint32_t be32toh(uint32_t big_endian_32bits);

// Endian conversion is symmetric, you convert between 2 endians, not from one to the other.
// SIDE NOTE: IPv6 address
// For IPv6, use struct sockaddr_in6 instead.

struct sockaddr_in6 {
  uint16_t sin6_family; // AF_INET6
  uint16_t sin6_port; // port in big-endian
  uint32_t sin6_flowinfo; // ignore
  struct in6_addr in6_addr; // IPv6
  uint32_t sin6_scope_id; // ignore
};
struct in6_addr {
  uint8_t s6_addr[16]; // IPv6
};

// struct sockaddr_in and struct sockaddr_in6 have different sizes, so the struct size (addrlen) is needed.
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// struct sockaddr is not used anywhere, just type cast struct sockaddr_in or struct sockaddr_in6 to this pointer type to match the function prototype.


/* Step 4: Listen
  All the previous steps are just passing paramets. The socket is actually created after listen().
  The OS will automatically handle the TCP handshakes and place established connections in a queue.
  The application can then retrieve them via accept().
*/

// listen
rv = listen(fd, SOMAXCONN);
if (rv) { die("listen()"); }

// The 2nd arugment is the size of the queue. SOMAXCONN is 4096 on Linux. This argument does not matter because accept() is not a bottleneck.




