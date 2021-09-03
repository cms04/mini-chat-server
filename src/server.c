#include <sys/socket.h>
#include <unistd.h>

#include "functions.h"
#include "server.h"

int init_server(char *username, char *ipaddr, uint16_t port, uint16_t bits) {
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
    RSA *key = create_rsa_key(bits);
    if (key == NULL) {
        CLOSE_SOCKET(fd_server);
        PRINT_ERROR("create_rsa_key");
    }
    printf("Ready to accept connections...\n");
    int fd_client_recv = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client_recv < 0) {
        CLOSE_SOCKET(fd_server);
        RSA_free(key);
        PRINT_ERROR("accept");
    }
    int fd_client_send = accept(fd_server, (struct sockaddr *) &client_addr, &client_len);
    if (fd_client_send < 0) {
        CLOSE_2_SOCKETS(fd_server, fd_client_recv);
        RSA_free(key);
        PRINT_ERROR("accept");
    }
    printf("Accepted one connection.\n");
    printf("Exchanging RSA public keys...\n");
    if (send_publickey(key, fd_client_send)) {
        CLOSE_2_SOCKETS(fd_server, fd_client_recv);
        RSA_free(key);
        PRINT_ERROR("send_publickey");
    }
    RSA* publickey = recv_publickey(fd_client_recv);
    if (publickey == NULL) {
        CLOSE_2_SOCKETS(fd_server, fd_client_recv);
        RSA_free(key);
        PRINT_ERROR("recv_publickey");
    }
    printf("RSA public keys exchanged\n");
    char *othername = get_message(fd_client_recv, key, publickey);
    if (othername == NULL) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        RSA_free(key);
        RSA_free(publickey);
        PRINT_ERROR("get_message");
    }
    printf("The other person's username is %s. Is this correct? [y,N] ", othername);
    char answer = getchar();
    getchar();
    if (send_message(fd_client_send, answer == 'Y' || answer == 'y' ? "Yes" : "No", publickey, key)) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        PRINT_ERROR("send_message");
    }
    if (!(answer == 'Y' || answer == 'y')) {
        printf("You denied the chat.\n");
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        return EXIT_SUCCESS;
    }
    if (send_message(fd_client_send, username, publickey, key)) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        PRINT_ERROR("send_message");
    }
    printf("Waiting for username verification...\n");
    char *recieved = get_message(fd_client_recv, key, publickey);
    if (recieved == NULL) {
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        PRINT_ERROR("get_message");
    }
    if (!strcmp(recieved, "No")) {
        printf("The other person denied the chat.\n");
        free(recieved);
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        return EXIT_SUCCESS;
    }
    free(recieved);
    printf("Both persons accepted the chat.\n");
    if (start_chat(fd_client_send, fd_client_recv, username, othername, publickey, key)) {
        free(othername);
        RSA_free(key);
        RSA_free(publickey);
        CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
        PRINT_ERROR("start_chat");
    }
    RSA_free(key);
    RSA_free(publickey);
    free(othername);
    CLOSE_3_SOCKETS(fd_client_recv, fd_client_send, fd_server);
    return EXIT_SUCCESS;
}
