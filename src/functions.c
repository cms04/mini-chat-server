#include <string.h>
#include <sys/socket.h>

#include "functions.h"

int send_message(int fd, char *msg) {
    size_t len = strlen(msg);
    char len_string[LEN_BUFFER_SIZE];
    bzero(len_string, LEN_BUFFER_SIZE);
    snprintf(len_string, LEN_BUFFER_SIZE - 1, "%ld", len);
    int bytes_sent = send(fd, len_string, strlen(len_string), 0);
    if (bytes_sent < 0) {
        PRINT_ERROR("send");
    }
    bzero(len_string, LEN_BUFFER_SIZE);
    int bytes_rcv = recv(fd, len_string, LEN_BUFFER_SIZE - 1, 0);
    if (bytes_rcv < 0) {
        PRINT_ERROR("recv");
    }
    if (atoi(len_string) != len) {
        PRINT_ERROR("send_message");
    }
    bytes_sent = send(fd, msg, strlen(msg), 0);
    if (bytes_sent < 0) {
        PRINT_ERROR("send");
    }
    return EXIT_SUCCESS;
}

char *get_message(int fd) {
    char len_string[LEN_BUFFER_SIZE];
    bzero(len_string, LEN_BUFFER_SIZE);
    int bytes_rcv = recv(fd, len_string, LEN_BUFFER_SIZE - 1, 0);
    if (bytes_rcv < 0) {
        PRINT_ERROR_RETURN_NULL("recv");
    }
    int bytes_sent = send(fd, len_string, strlen(len_string), 0);
    if (bytes_sent < 0) {
        PRINT_ERROR_RETURN_NULL("send");
    }
    size_t len = atoi(len_string);
    char *msg = (char *) malloc((len + 1) * sizeof(char));
    if (msg == NULL) {
        PRINT_ERROR_RETURN_NULL("malloc");
    }
    bzero(msg, len + 1);
    bytes_rcv = recv(fd, msg, len, 0);
    if (bytes_rcv < 0) {
        free(msg);
        PRINT_ERROR_RETURN_NULL("recv");
    }
    return msg;
}
