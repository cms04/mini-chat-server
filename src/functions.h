#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LEN_BUFFER_SIZE 15

#define PRINT_ERROR(function) fprintf(stderr, "ERROR: %s() failed: %s\n", function, strerror(errno)); \
                              return EXIT_FAILURE

#define PRINT_ERROR_RETURN_NULL(function) fprintf(stderr, "ERROR: %s() failed: %s\n", function, strerror(errno)); \
                                          return NULL

#define CLOSE_SOCKET(fd) shutdown(fd, SHUT_RDWR); \
                         close(fd)

#define CLOSE_2_SOCKETS(fd1, fd2) CLOSE_SOCKET(fd1); \
                                  CLOSE_SOCKET(fd2)

#define CLOSE_3_SOCKETS(fd1, fd2, fd3) CLOSE_2_SOCKETS(fd1, fd2); \
                                       CLOSE_SOCKET(fd3)

#define INIT_SOCKADDR_STRUCT(str, ipaddr, port) bzero(&str, sizeof(str)); \
                                                str.sin_family = AF_INET; \
                                                str.sin_addr.s_addr = inet_addr(ipaddr); \
                                                str.sin_port = htons(port)

int send_message(int fd, char *msg);
char *get_message(int fd);

#endif
