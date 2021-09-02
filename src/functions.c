#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <unistd.h>

#include "functions.h"

int send_message(int fd, char *msg, RSA *publickey) {
    size_t rsa_size = RSA_size(publickey);
    size_t block_size = rsa_size - 42;
    size_t msg_len = strlen(msg);
    size_t block_count = msg_len / block_size + 1;
    char len_string[LEN_BUFFER_SIZE];
    bzero(len_string, LEN_BUFFER_SIZE);
    snprintf(len_string, LEN_BUFFER_SIZE - 1, "%ld", block_count);
    int bytes_sent = send(fd, len_string, strlen(len_string), 0);
    if (bytes_sent < 0) {
        PRINT_ERROR("send");
    }
    bzero(len_string, LEN_BUFFER_SIZE);
    int bytes_rcv = recv(fd, len_string, LEN_BUFFER_SIZE - 1, 0);
    if (bytes_rcv < 0) {
        PRINT_ERROR("recv");
    }
    if (strtoul(len_string, NULL, 10) != block_count) {
        PRINT_ERROR("send_message");
    }
    char *crypt = (char *) malloc(rsa_size * sizeof(char));
    bzero(crypt, rsa_size);
    char *block = (char *) malloc(block_size * sizeof(char));
    bzero(block, block_size);
    char *msg_ptr = msg;
    for (size_t i = 0; i < block_count; i++) {
        strncpy(block, msg_ptr, block_size);
        if (RSA_public_encrypt(block_size, (unsigned char *) block, (unsigned char *) crypt, publickey, RSA_PKCS1_OAEP_PADDING) < 0) {
            free(crypt);
            free(block);
            ERROR_OPENSSL("RSA_public_encrypt");
        }
        if (send(fd, crypt, rsa_size, 0) < 0) {
            free(crypt);
            free(block);
            PRINT_ERROR("send");
        }
        msg_ptr += block_size;
    }
    free(block);
    free(crypt);
    return EXIT_SUCCESS;
}

char *get_message(int fd, RSA *privatekey) {
    size_t rsa_size = RSA_size(privatekey);
    char len_string[LEN_BUFFER_SIZE];
    bzero(len_string, LEN_BUFFER_SIZE);
    int bytes_rcv = recv(fd, len_string, LEN_BUFFER_SIZE - 1, 0);
    if (bytes_rcv < 0) {
        PRINT_ERROR_RETURN_NULL("recv");
    }
    int bytes_sent = send(fd, len_string, strlen(len_string), 0);
    if (bytes_sent < 0) {
        PRINT_ERROR_RETURN_NULL("send");
    }
    size_t block_count = atoi(len_string);
    char *msg = (char *) malloc(rsa_size * block_count * sizeof(char));
    if (msg == NULL) {
        PRINT_ERROR_RETURN_NULL("malloc");
    }
    bzero(msg, rsa_size * block_count);
    char *crypt = (char *) malloc(sizeof(char) * rsa_size);
    if (crypt == NULL) {
        free(msg);
        PRINT_ERROR_RETURN_NULL("malloc");
    }
    char *msg_ptr = msg;
    size_t block_size = rsa_size - 42;
    for (size_t i = 0; i < block_count; i++) {
        if (recv(fd, crypt, rsa_size, 0) < 0) {
            free(crypt);
            free(msg);
            PRINT_ERROR_RETURN_NULL("recv");
        }
        if (RSA_private_decrypt(rsa_size, (unsigned char *) crypt, (unsigned char *) msg_ptr, privatekey, RSA_PKCS1_OAEP_PADDING) < 0) {
            free(crypt);
            free(msg);
            ERROR_OPENSSL_RETURN_NULL("RSA_private_decrypt");
        }
        msg_ptr += block_size;
    }
    free(crypt);
    return msg;
}

int start_chat(int fd_send, int fd_recv, char *username, char *othername, RSA *publickey, RSA *privatekey) {
    thread_parameter_t *p = (thread_parameter_t *) malloc(sizeof(thread_parameter_t));
    if (p == NULL) {
        PRINT_ERROR("malloc");
    }
    p->fd_recv = fd_recv;
    p->fd_send = fd_send;
    p->username = username;
    p->othername = othername;
    p->publickey = publickey;
    p->privatekey = privatekey;
    p->enabled = 1;
    pthread_t send_thread_s, recv_thread_s;
    if (pthread_create(&send_thread_s, NULL, &send_thread, (void *) p)) {
        free(p);
        PRINT_ERROR("pthread_create");
    }
    if (pthread_create(&recv_thread_s, NULL, &recv_thread, (void *) p)) {
        free(p);
        PRINT_ERROR("pthread_create");
    }
    printf("Ready to chat.\n\n");
    long status = EXIT_SUCCESS, tmp = EXIT_SUCCESS;
    if (pthread_join(recv_thread_s, (void *) ((long *) &status))) {
        free(p);
        PRINT_ERROR("pthread_join");
    }
    if (pthread_join(send_thread_s, (void *) ((long *) &tmp))) {
        free(p);
        PRINT_ERROR("pthread_join");
    }
    free(p);
    return (int) (status | tmp);
}


char *read_input(void) {
    char *result = NULL;
    size_t n = 0;
    if (getline(&result, &n, stdin) < 0) {
        free(result);
        return NULL;
    }
    return result;
}

void *send_thread(void *ptr) {
    thread_parameter_t *p = (thread_parameter_t *) ptr;
    while (p->enabled) {
        char *msg = read_input();
        if (msg == NULL) {
            p->enabled = 0;
            PRINT_ERROR_PTHREAD("read_input");
        }
        if (!strcmp(msg, "\n")) {
            printf("\033[A\r");
            free(msg);
            continue;
        }
        if (send_message(p->fd_send, msg, p->publickey)) {
            p->enabled = 0;
            free(msg);
            PRINT_ERROR_PTHREAD("send_message");
        }
        if (!strcmp(msg, "quit_chat\n")) {
            p->enabled = 0;
            printf("\nYou have left the chat.\n"
                   "Wait for %s to close the chat...\n", p->othername);
        } else {
            printf("\033[A\r[ %s ] %s", p->username, msg);
        }
        free(msg);
    }
    pthread_exit((void *) ((long) EXIT_SUCCESS));
}

void *recv_thread(void *ptr) {
    thread_parameter_t *p = (thread_parameter_t *) ptr;
    while (p->enabled) {
        char *msg = get_message(p->fd_recv, p->privatekey);
        if (msg == NULL) {
            p->enabled = 0;
            PRINT_ERROR_PTHREAD("get_message");
        }
        if (!strcmp(msg, "quit_chat\n")) {
            p->enabled = 0;
            printf("\n%s has left the chat.\n"
                   "Press [ENTER] to close the chat.\n", p->othername);
        } else {
            printf("[ %s ] %s", p->othername, msg);
        }
        free(msg);
    }
    pthread_exit((void *) ((long) EXIT_SUCCESS));
}

RSA *create_rsa_key(uint16_t bits) {
    printf("Your RSA key is generated...\n");
    RSA *key = RSA_new();
    if (key == NULL) {
        ERROR_OPENSSL_RETURN_NULL("RSA_new");
    }
    srand(time(NULL));
    BIGNUM *e = BN_new();
    if (e == NULL) {
        RSA_free(key);
        ERROR_OPENSSL_RETURN_NULL("BN_new");
    }
    if (!BN_set_word(e, RSA_F4)) {
        RSA_free(key);
        BN_clear_free(e);
        ERROR_OPENSSL_RETURN_NULL("BN_rand");
    }
    if (!RSA_generate_key_ex(key, bits, e, NULL)) {
        RSA_free(key);
        BN_clear_free(e);
        ERROR_OPENSSL_RETURN_NULL("RSA_generate_key_ex");
    }
    BN_clear_free(e);
    printf("Your RSA key was generated successfully.\n");
    return key;
}

int send_publickey(RSA *key, int fd_send) {
    RSA *publickey = RSAPublicKey_dup(key);
    if (publickey == NULL) {
        ERROR_OPENSSL("RSAPublicKey_dup");
    }
    FILE *fp = fopen("sended.key", "w+");
    if (fp == NULL) {
        RSA_free(publickey);
        PRINT_ERROR("fopen");
    }
    PEM_write_RSAPublicKey(fp, publickey);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char buf[len];
    bzero(buf, len);
    fread(buf, sizeof(char), len, fp);
    int bytes_sent = send(fd_send, buf, len, 0);
    if (bytes_sent < 0) {
        PRINT_ERROR("send");
    }
    fclose(fp);
    unlink("sended.key");
    RSA_free(publickey);
    return EXIT_SUCCESS;
}

RSA *recv_publickey(int fd_recv) {
    char buf[4096];
    bzero(buf, 4096);
    int bytes_rcv = recv(fd_recv, buf, 4096, 0);
    if (bytes_rcv < 0) {
        PRINT_ERROR_RETURN_NULL("recv");
    }
    FILE *fp = fopen("recieved.key", "w+");
    if (fp == NULL) {
        PRINT_ERROR_RETURN_NULL("fopen");
    }
    fwrite(buf, sizeof(char), bytes_rcv, fp);
    fseek(fp, 0, SEEK_SET);
    RSA *publickey = RSA_new();
    if (publickey == NULL) {
        fclose(fp);
        unlink("recieved.key");
        ERROR_OPENSSL_RETURN_NULL("RSA_new");
    }
    if (PEM_read_RSAPublicKey(fp, &publickey, NULL, NULL) == NULL) {
        RSA_free(publickey);
        fclose(fp);
        unlink("recieved.key");
        ERROR_OPENSSL_RETURN_NULL("PEM_read_RSAPublicKey");
    }
    fclose(fp);
    unlink("recieved.key");
    return publickey;
}
