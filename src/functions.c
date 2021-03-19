#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

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
    if (strtoul(len_string, NULL, 10) != len) {
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

int start_chat(int fd_send, int fd_recv, char *username, char *othername) {
    thread_parameter_t *p = (thread_parameter_t *) malloc(sizeof(thread_parameter_t));
    if (p == NULL) {
        PRINT_ERROR("malloc");
    }
    p->fd_recv = fd_recv;
    p->fd_send = fd_send;
    p->username = username;
    p->othername = othername;
    p->enabled = 1;
    pthread_t send_thread_s, recv_thread_s;
    if (pthread_create(&send_thread_s, NULL, &send_thread, (void *) p)) {
        free(p);
        PRINT_ERROR("pthread_create");
    }
    if (pthread_create(&recv_thread_s, NULL, &recv_thread, (void *) p)) {
        free(p);
        PRINT_ERROR("pthread_create");
    }
    printf("Ready to chat.\n\n");
    int status = EXIT_SUCCESS, tmp = EXIT_SUCCESS;
    if (pthread_join(recv_thread_s, (void *) ((long *) &status))) {
        free(p);
        PRINT_ERROR("pthread_join");
    }
    if (pthread_join(send_thread_s, (void *) ((long *) &tmp))) {
        free(p);
        PRINT_ERROR("pthread_join");
    }
    free(p);
    return status | tmp;
}

void *send_thread(void *ptr) {
    thread_parameter_t *p = (thread_parameter_t *) ptr;
    while (p->enabled) {
        char msg_buffer[TEXT_BUFFER_SIZE];
        bzero(msg_buffer, TEXT_BUFFER_SIZE);
        fgets(msg_buffer, TEXT_BUFFER_SIZE - 1, stdin);
        if (send_message(p->fd_send, msg_buffer)) {
            p->enabled = 0;
            PRINT_ERROR_PTHREAD("send_message");
        }
        if (!strcmp(msg_buffer, "quit_chat\n")) {
            p->enabled = 0;
            printf("\nYou have left the chat.\n");
        } else {
            printf("\033[A\r[ %s ] %s", p->username, msg_buffer);
        }
    }
    pthread_exit((void *) ((long) EXIT_SUCCESS));
}

void *recv_thread(void *ptr) {
    thread_parameter_t *p = (thread_parameter_t *) ptr;
    while (p->enabled) {
        char *msg = get_message(p->fd_recv);
        if (msg == NULL) {
            p->enabled = 0;
            PRINT_ERROR_PTHREAD("get_message");
        }
        if (!strcmp(msg, "quit_chat\n")) {
            p->enabled = 0;
            printf("\n%s has left the chat.\n"
                   "Press [ENTER] to close the chat.\n", p->othername);
        } else {
            printf("[ %s ] %s", p->othername, msg);
        }
        free(msg);
    }
    pthread_exit((void *) ((long) EXIT_SUCCESS));
}
