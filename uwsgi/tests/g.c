#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

int main(int argc, char** argv) {
  struct addrinfo* result;
  struct addrinfo* res;
  int error;

  /* resolve the domain name into a list of addresses */
  error = getaddrinfo(argv[1], NULL, NULL, &result);
  if (error != 0) {
    if (error == EAI_SYSTEM) {
      perror("getaddrinfo");
    } else {
      fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
    }
    exit(EXIT_FAILURE);
  }
  char str[INET6_ADDRSTRLEN] = {0};
  switch (result->ai_family) {
    case AF_INET: {
      inet_ntop(AF_INET, &( ((struct sockaddr_in*)(result->ai_addr))->sin_addr ), str, INET_ADDRSTRLEN);
      puts(str);
      break;
    }
    case AF_INET6:
      inet_ntop(AF_INET6, &( ((struct sockaddr_in*)(result->ai_addr))->sin_addr ), str, INET6_ADDRSTRLEN);
      puts(str);
      break;
    default:
      break;
  }

  freeaddrinfo(result);
  return 0;
}
