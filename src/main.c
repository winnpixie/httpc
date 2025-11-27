#include "httpsrv.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// FIXME: Surely there is exists a not as dumb looking solution?
void *srv_thread(void *args) {
  start_server(((int *)args)[0]);
  return NULL;
}

int main(int argc, char **argv) {
  int srvport = 8080;
  if (argc > 1) {
    srvport = atoi(argv[1]);
  }

  int srvsockfd = create_server(srvport);
  if (srvsockfd < 0) {
    printf("Error creating HTTP server on port %d\n", srvport);
    return EXIT_FAILURE;
  }

  printf("Starting HTTP server on port %d\n", srvport);

  pthread_t srvthread;
  int *thrdargs = malloc(sizeof(int));
  thrdargs[0] = srvsockfd;
  pthread_create(&srvthread, NULL, srv_thread, thrdargs);
  free(thrdargs);

  while (getchar() != 'x') {
  }
  printf("Stop requested\n");

  stop_server();
  pthread_join(srvthread, NULL);

  printf("Exiting.\n");
  return EXIT_SUCCESS;
}