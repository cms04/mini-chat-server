#include <sys/socket.h>
#include <unistd.h>

#include "functions.h"
#include "client.h"

int init_client(char *username, char *ipaddr, uint16_t port, uint16_t bits) {
    int fd_client_send = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_client_send < 0) {
        PRINT_ERROR("socket");
    }
    int fd_client_recv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_client_recv < 0) {
        CLOSE_SOCKET(fd_client_send);
        PRINT_ERROR("socket");
    }
    RSA *key = create_rsa_key(bits);
    if (key == NULL) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("create_rsa_key");
    }
    struct sockaddr_in server_addr;
    INIT_SOCKADDR_STRUCT(server_addr, ipaddr, port);
    if (connect(fd_client_send, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(key);
        PRINT_ERROR("connect");
    }
    if (connect(fd_client_recv, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(key);
        PRINT_ERROR("connect");
    }
    printf("Connection successful.\n");
    printf("Exchanging RSA public keys...\n");
    RSA *publickey = recv_publickey(fd_client_recv);
    if (publickey == NULL) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(key);
        PRINT_ERROR("recv_publickey");
    }
    if (send_publickey(key, fd_client_send)) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(key);
        RSA_free(publickey);
        PRINT_ERROR("recv_publickey");
    }
    printf("RSA public keys exchanged\n");
    printf("Waiting for username verification...\n");
    if (send_message(fd_client_send, username)) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(publickey);
        RSA_free(key);
        PRINT_ERROR("send_message");
    }
    char *recieved = get_message(fd_client_recv);
    if (recieved == NULL) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(publickey);
        RSA_free(key);
        PRINT_ERROR("get_message");
    }
    if (!strcmp(recieved, "No")) {
        printf("The other person denied the chat.\n");
        RSA_free(publickey);
        RSA_free(key);
        free(recieved);
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        return EXIT_SUCCESS;
    }
    free(recieved);
    char *othername = get_message(fd_client_recv);
    if (othername == NULL) {
        RSA_free(publickey);
        RSA_free(key);
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("get_message");
    }
    printf("The other person's username is %s. Is this correct? [y,N] ", othername);
    char answer = getchar();
    getchar();
    if (send_message(fd_client_send, answer == 'Y' || answer == 'y' ? "Yes" : "No")) {
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(publickey);
        RSA_free(key);
        free(othername);
        PRINT_ERROR("send_message");
    }
    if (!(answer == 'Y' || answer == 'y')) {
        printf("You denied the chat.\n");
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        RSA_free(publickey);
        RSA_free(key);
        free(othername);
        return EXIT_SUCCESS;
    }
    printf("Both persons accepted the chat.\n");
    if(start_chat(fd_client_send, fd_client_recv, username, othername)) {
        RSA_free(publickey);
        RSA_free(key);
        free(othername);
        CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
        PRINT_ERROR("start_chat");
    }
    free(othername);
    RSA_free(publickey);
    RSA_free(key);
    CLOSE_2_SOCKETS(fd_client_recv, fd_client_send);
    return EXIT_SUCCESS;
}
