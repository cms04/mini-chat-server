#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "functions.h"
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
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("connect");
    }
    char *msg = "Hallo";
    if (send_message(fd_client, msg)) {
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("send_message");
    }
    char *recieved = get_message(fd_client);
    if (recieved == NULL) {
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("get_message");
    }
    printf("%s\n", recieved);
    free(recieved);
    CLOSE_SOCKET(fd_client);
    return EXIT_SUCCESS;
}
