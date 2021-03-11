#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "functions.h"
#include "server.h"

int init_server(char *username, char *ipaddr, uint16_t port) {
    int fd_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_server == 0) {
        PRINT_ERROR("socket");
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipaddr);
    server_addr.sin_port = htons(port);
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
    int fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client < 0) {
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("accept");
    }
    char *recieved = get_message(fd_client);
    if (recieved == NULL) {
        CLOSE_SOCKET(fd_client);
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("get_message");
    }
    printf("%s\n", recieved);
    free(recieved);
    char *msg = "Welt";
    if (send_message(fd_client, msg)) {
        CLOSE_SOCKET(fd_client);
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("send_message");
    }
    CLOSE_SOCKET(fd_client);
    CLOSE_SOCKET(fd_server);
    return EXIT_SUCCESS;
}
