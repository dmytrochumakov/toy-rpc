#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "isPrime.h"

#define SERVERPORT "5005"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

int unpack(int packedInput) { return ntohs(packedInput); }

int getAndBindSocket() {
  int sockfd;
  struct addrinfo hints, *serverInfo, *p;
  int numberOfBytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int returnValue = getaddrinfo(NULL, SERVERPORT, &hints, &serverInfo);
  if (returnValue != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returnValue));
    exit(1);
  }

  for (p = serverInfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror(" setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(2);
  }

  return sockfd;
}

int main(void) {
  int sockfd = getAndBindSocket();

  if (listen(sockfd, 1) == -1) {
    perror("listen");
    exit(1);
  }

  printf("Server waiting for connections.\n");

  struct sockaddr theirAddr;
  socklen_t sinSize;
  int newfd;

  while (1) {
    sinSize = sizeof theirAddr;
    newfd = accept(sockfd, (struct sockaddr *)&theirAddr, &sinSize);

    if (newfd == -1) {
      perror("accept");
      continue;
    }

    int buffer;
    int bytesReceived = recv(newfd, &buffer, sizeof buffer, 0);

    if (bytesReceived == -1) {
      perror("recv");
      continue;
    }

    int number = unpack(buffer);
    printf("Received a request: is %d prime?\n", number);

    bool numberIsPrime = isPrime(number);
    printf("Sending response: %s\n", numberIsPrime ? "true" : "false");

    if (send(newfd, &numberIsPrime, sizeof numberIsPrime, 0) == -1) {
      perror("send");
    }

    close(newfd);
  }
}
