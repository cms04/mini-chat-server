#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "macros.h"
#include "client.h"

int init_client(char *username, char *ipaddr, uint16_t port) {
    int fd_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_client < 0) {
        PRINT_ERROR("socket");
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipaddr);
    server_addr.sin_port = htons(port);
    if (connect(fd_client, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        close(fd_client);
        PRINT_ERROR("connect");
    }
    char *msg = "Hallo";
    if (send(fd_client, msg, strlen(msg), 0) < 0) {
        close(fd_client);
        PRINT_ERROR("send");
    }
    char buf[10];
    int bytes_rcv = recv(fd_client, buf, 9, 0);
    if (bytes_rcv < 0) {
        close(fd_client);
        PRINT_ERROR("recv");
    }
    buf[bytes_rcv] = '\0';
    printf("%s\n", buf);
    close(fd_client);
    return EXIT_SUCCESS;
}
