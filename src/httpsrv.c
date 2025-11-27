#include "httpsrv.h"
#include "client.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>

int state = 0;

int prepare_server(const int srvsockfd, const int srvport) {
  struct sockaddr_in srvaddr;

  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons(srvport);
  srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(srvsockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1) {
    perror("Error - bind()");
    return -1;
  }

  if (listen(srvsockfd, 5) == -1) {
    perror("Error - listen()");
    return -2;
  }

  return 0;
}

int create_server(const int port) {
  int srvsockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (srvsockfd == -1) {
    perror("Error - socket()");
    return -1;
  }

  if (prepare_server(srvsockfd, port) < 0) {
    perror("Error - configuring");
    return -2;
  }

  return srvsockfd;
}

void start_server(const int srvsockfd) {
  state = 1;

  while (state == 1) {
    struct sockaddr claddr;
    socklen_t claddrlen;

    int clsockfd = accept(srvsockfd, &claddr, &claddrlen);
    if (clsockfd != -1) {
      handle_client(clsockfd);
    }
  }

  shutdown(srvsockfd, SHUT_RDWR);
}

void stop_server() {
  state = 0;
}