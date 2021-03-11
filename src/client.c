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
    if (send_message(fd_client, username)) {
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("send_message");
    }
    char *recieved = get_message(fd_client);
    if (recieved == NULL) {
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("get_message");
    }
    if (!strcmp(recieved, "No")) {
        printf("The other person denied the chat.\n");
        free(recieved);
        CLOSE_SOCKET(fd_client);
        return EXIT_SUCCESS;
    }
    free(recieved);
    recieved = get_message(fd_client);
    if (recieved == NULL) {
        CLOSE_SOCKET(fd_client);
        PRINT_ERROR("get_message");
    }
    printf("The other person's username is %s. Is this correct? [y,N] ", recieved);
    free(recieved);
    char answer = getchar();
    if (answer == 'Y' || answer == 'y') {
        if (send_message(fd_client, "Yes")) {
            CLOSE_SOCKET(fd_client);
            PRINT_ERROR("send_message");
        }
    } else {
        printf("You denied the chat.\n");
        if (send_message(fd_client, "No")) {
            CLOSE_SOCKET(fd_client);
            PRINT_ERROR("send_message");
        }
        CLOSE_SOCKET(fd_client);
        return EXIT_SUCCESS;
    }
    printf("Both persons accepted the chat.\n");
    CLOSE_SOCKET(fd_client);
    return EXIT_SUCCESS;
}
