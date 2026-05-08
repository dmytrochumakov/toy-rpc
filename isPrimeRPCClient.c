#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVERPORT "5005"  // The port the server will be listening on.
#define SERVER "localhost" // Assume localhost for now

#include "isPrimeRPCClient.h"

int pack(int input) { return htons(input); }

void *getInAddr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

int getSocket() {
  int sockfd;
  struct addrinfo hints, *serverInfo, *p;
  int numberOfBytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int returnValue = getaddrinfo(SERVER, SERVERPORT, &hints, &serverInfo);
  if (returnValue != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returnValue));
    exit(1);
  }

  for (p = serverInfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if ((connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    exit(2);
  }

  return sockfd;
}

bool isPrimeRPC(int number) {
  int packedNumber = pack(number);
  int sockfd = getSocket();

  if (send(sockfd, &packedNumber, sizeof packedNumber, 0) == -1) {
    perror("send");
    close(sockfd);
    exit(0);
  }

  int buf[1];
  int bytesReceived = recv(sockfd, &buf, 1, 0);
  if (bytesReceived == -1) {
    perror("recv");
    exit(1);
  }

  bool result = buf[0];

  close(sockfd);
  return result;
}
// pack, connect to server, send data, receive data, unpack and return it
