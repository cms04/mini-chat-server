#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/rsa.h>
#include <pthread.h>

typedef struct {
    int fd_send;
    int fd_recv;
    short enabled;
    char *username;
    char *othername;
} thread_parameter_t;

#define LEN_BUFFER_SIZE 15
#define TEXT_BUFFER_SIZE 50

#define PRINT_ERROR(function) fprintf(stderr, "ERROR: %s() failed at %s, line %d: %s\n", function, __FILE__, __LINE__, strerror(errno)); \
                              return EXIT_FAILURE

#define PRINT_ERROR_RETURN_NULL(function) fprintf(stderr, "ERROR: %s() failed at %s, line %d: %s\n", function, __FILE__, __LINE__, strerror(errno)); \
                                          return NULL

#define PRINT_ERROR_PTHREAD(function) fprintf(stderr, "ERROR: %s() failed at %s, line %d: %s\n", function, __FILE__, __LINE__, strerror(errno)); \
                                      pthread_exit((void *) ((long) EXIT_FAILURE))

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
int start_chat(int fd_send, int fd_recv, char *username, char *othername);
char *read_input(void);

void *send_thread(void *ptr);
void *recv_thread(void *ptr);
RSA *create_rsa_key(void);
int send_publickey(RSA *key, int fd_send);
RSA *recv_publickey(int fd_recv);

#endif
