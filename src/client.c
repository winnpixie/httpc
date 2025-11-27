#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void write_headers(const int clsockfd, const char *status, const char *contenttype, const long contentlen) {
  char *head = malloc(sizeof(char) * 128);
  sprintf(head,
    "HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %ld\nConnection: close\n\n",
    status, contenttype, contentlen);

  send(clsockfd, head, strlen(head), 0);
  free(head);
}

void handle_client(const int clsockfd) {
  size_t bufsize = sizeof(char) * 1024;
  char *reqbuf = malloc(bufsize);
  recv(clsockfd, reqbuf, bufsize, 0);

  printf("%s\n", reqbuf);
  char *reqmethod = strtok(reqbuf, " ");
  if (strcmp(reqmethod, "GET") == 0) {
    char *reqpath = strtok(reqbuf + 4, " ");
    if (strlen(reqpath) == 1) {
      reqpath = "/index.html";
    }

    FILE *reqfile = fopen(reqpath + 1, "r");
    if (reqfile == NULL) {
      write_headers(clsockfd, "404 Not Found", "text/plain", 4);
      send(clsockfd, "404.", 4, 0);
    } else {
      fseek(reqfile, 0, SEEK_END);
      size_t filesize = sizeof(char) * ftell(reqfile);
      rewind(reqfile);

      char *filebuf = malloc(filesize);
      fread(filebuf, sizeof(char), filesize, reqfile);

      fclose(reqfile);

      write_headers(clsockfd, "200 OK", "text/html", filesize);
      send(clsockfd, filebuf, filesize, 0);

      free(filebuf);
    }
  } else {
    printf("Error processing request.\n");
  }

  free(reqbuf);
  shutdown(clsockfd, SHUT_RDWR);
}