#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>

#include "server.h"
#include "client.h"

int main(int argc, char *const *argv) {
    char *username = NULL, *ipaddr = NULL;
    uint16_t port = 0;
    uint16_t bits = 2048;
    uint8_t is_server = 0;
    extern char *optarg;
    char param = -1;
    while ((param = getopt(argc, argv, "u:a:p:b:s")) != EOF) {
        switch (param) {
            case 'u':
                username = optarg;
                break;
            case 'a':
                ipaddr = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                is_server = 1;
                break;
            case 'b':
                bits = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Invalid parameter -%c\n", param);
                break;
        }
    }
    if (username == NULL || ipaddr == NULL || port == 0) {
        fprintf(stderr, "ERROR: You have to set a username, an IP-adress and a port.\n");
        return EXIT_FAILURE;
    }
    if (bits != 1024 && bits != 2048 && bits != 4096 && bits != 8192) {
        fprintf(stderr, "ERROR: RSA bit size has to be 1024, 2048, 4096 or 8192.\n");
        return EXIT_FAILURE;
    }
    int status = is_server ? init_server(username, ipaddr, port, bits)
                           : init_client(username, ipaddr, port, bits);
    return status;
}
