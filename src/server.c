#include <sys/socket.h>
#include <unistd.h>

#include "functions.h"
#include "server.h"

int init_server(char *username, char *ipaddr, uint16_t port) {
    int fd_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_server == 0) {
        PRINT_ERROR("socket");
    }
    struct sockaddr_in server_addr;
    INIT_SOCKADDR_STRUCT(server_addr, ipaddr, port);
    if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("bind");
    }
    if (listen(fd_server, 0) < 0) {
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("listen");
    }
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd_client_recv = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client_recv < 0) {
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("accept");
    }
    int fd_client_send = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client_send < 0) {
        CLOSE_2_SOCKETS(fd_server, fd_client_recv);
        PRINT_ERROR("accept");
    }
    char *othername = get_message(fd_client_recv);
    if (othername == NULL) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        PRINT_ERROR("get_message");
    }
    printf("The other person's username is %s. Is this correct? [y,N] ", othername);
    char answer = getchar();
    if (send_message(fd_client_send, answer == 'Y' || answer == 'y' ? "Yes" : "No")) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        PRINT_ERROR("send_message");
    }
    if (!(answer == 'Y' || answer == 'y')) {
        printf("You denied the chat.\n");
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        return EXIT_SUCCESS;
    }
    if (send_message(fd_client_send, username)) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        PRINT_ERROR("send_message");
    }
    char *recieved = get_message(fd_client_recv);
    if (recieved == NULL) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        PRINT_ERROR("get_message");
    }
    if (!strcmp(recieved, "No")) {
        printf("The other person denied the chat.\n");
        free(recieved);
        free(othername);
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        return EXIT_SUCCESS;
    }
    free(recieved);
    printf("Both persons accepted the chat.\n");
    if (start_chat(fd_client_send, fd_client_recv, username, othername)) {
        free(othername);
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        PRINT_ERROR("start_chat");
    }
    free(othername);
    CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
    return EXIT_SUCCESS;
}
