#include <sys/socket.h>
#include <unistd.h>

#include "functions.h"
#include "client.h"

int init_client(char *username, char *ipaddr, uint16_t port) {
    int fd_client_send = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_client_send < 0) {
        PRINT_ERROR("socket");
    }
    int fd_client_recv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_client_recv < 0) {
        CLOSE_SOCKET(fd_client_send);
        PRINT_ERROR("socket");
    }
    struct sockaddr_in server_addr;
    INIT_SOCKADDR_STRUCT(server_addr, ipaddr, port);
    if (connect(fd_client_send, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("connect");
    }
    if (connect(fd_client_recv, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("connect");
    }
    if (send_message(fd_client_send, username)) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("send_message");
    }
    char *recieved = get_message(fd_client_recv);
    if (recieved == NULL) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("get_message");
    }
    if (!strcmp(recieved, "No")) {
        printf("The other person denied the chat.\n");
        free(recieved);
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        return EXIT_SUCCESS;
    }
    free(recieved);
    char *othername = get_message(fd_client_recv);
    if (othername == NULL) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("get_message");
    }
    printf("The other person's username is %s. Is this correct? [y,N] ", othername);
    char answer = getchar();
    if (send_message(fd_client_send, answer == 'Y' || answer == 'y' ? "Yes" : "No")) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        free(othername);
        PRINT_ERROR("send_message");
    }
    if (!(answer == 'Y' || answer == 'y')) {
        printf("You denied the chat.\n");
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        free(othername);
        return EXIT_SUCCESS;
    }
    printf("Both persons accepted the chat.\n");
    if(start_chat(fd_client_send, fd_client_recv, username, othername)) {
        free(othername);
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("start_chat");
    }
    free(othername);
    CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
    return EXIT_SUCCESS;
}
