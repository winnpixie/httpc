#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void write_headers(const int clsockfd, const char *status, const char *contenttype, const long contentlen) {
  size_t maxhead = sizeof(char) * 256;
  char *head = malloc(maxhead);
  int headlen = snprintf(head, maxhead,
            "HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %ld\nConnection: close\n\n",
            status, contenttype, contentlen);
  
  if (headlen == -1 || headlen >= maxhead) {
    free(head);
    return;
  };

  send(clsockfd, head, headlen, 0);
  free(head);
}

void finish_client(const int clsockfd) {
  shutdown(clsockfd, SHUT_RDWR);
  close(clsockfd);
}

int is_valid_http_method(const char *method) {
  if (method == NULL)
    return 0;

  if (strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0) {
    return 1;
  }

  return 0;
}

void handle_client(const int clsockfd) {
  size_t bufsize = sizeof(char) * 1024;
  char *reqbuf = malloc(bufsize);
  if (reqbuf == NULL) {
    finish_client(clsockfd);

    return;
  }

  ssize_t reqread = recv(clsockfd, reqbuf, bufsize - 1, 0);
  if (reqread < 1) {
    free(reqbuf);
    finish_client(clsockfd);

    return;
  }

  reqbuf[reqread] = '\0';

  printf("%s\n", reqbuf);

  char *tokptr = NULL;
  char *reqmethod = strtok_r(reqbuf, " ", &tokptr);
  char *reqpath = strtok_r(NULL, " ", &tokptr);

  if (is_valid_http_method(reqmethod) == 1) {
    if (reqpath == NULL || strcmp(reqpath, "/") == 0) {
      reqpath = "/index.html";
    }

    FILE *reqfile = fopen(reqpath + 1, "rb");
    if (reqfile == NULL) {
      write_headers(clsockfd, "404 Not Found", "text/plain", 4);
      send(clsockfd, "404.", 4, 0);
    } else {
      if (fseek(reqfile, 0L, SEEK_END) == 0) {
        long filesize = ftell(reqfile);
        rewind(reqfile);

        if (filesize == 0) {
          write_headers(clsockfd, "200 OK", "text/plain", 0L);
        } else if (filesize > 0) {
          ssize_t respsize = sizeof(char) * filesize;
          char *respbuf = malloc(respsize);
          if (respbuf == NULL) {
            write_headers(clsockfd, "500 Internal Server Error", "text/plain", 4);
            send(clsockfd, "500.", 4, 0);
          } else {
            size_t respread = fread(respbuf, sizeof(char), filesize, reqfile);
            if (respread == filesize) {
              write_headers(clsockfd, "200 OK", "text/html", filesize);
              send(clsockfd, respbuf, filesize, 0);
            }
            
            free(respbuf);
          }
        } else {
          write_headers(clsockfd, "500 Internal Server Error", "text/plain", 4L);
          send(clsockfd, "500.", 4, 0);
        }
      }

      fclose(reqfile);
    }
  } else {
    write_headers(clsockfd, "400 Bad Request", "text/plain", 4L);
    send(clsockfd, "400.", 4, 0);
  }

  free(reqbuf);
  finish_client(clsockfd);
}