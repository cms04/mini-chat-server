#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "macros.h"
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
        close(fd_server);
        PRINT_ERROR("bind");
    }
    if (listen(fd_server, 0) < 0) {
        close(fd_server);
        PRINT_ERROR("listen");
    }
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client < 0) {
        close(fd_server);
        PRINT_ERROR("accept");
    }

    char buf[10];
    int bytes_rcv = recv(fd_client, buf, 9, 0);
    if (bytes_rcv < 0) {
        close(fd_client);
        close(fd_server);
        PRINT_ERROR("recv");
    }
    buf[bytes_rcv] = '\0';
    printf("%s\n", buf);
    char *msg = "Welt";
    int bytes_sent = send(fd_client, msg, strlen(msg), 0);
    if (bytes_sent < 0) {
        close(fd_client);
        close(fd_server);
        PRINT_ERROR("send");
    }
    close(fd_client);
    close(fd_server);
    return EXIT_SUCCESS;
}
