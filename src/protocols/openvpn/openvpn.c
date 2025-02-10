#include "openvpn.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "src/network/network.h"
#include "src/util/print.h"

void end_connection(SSL *ssl) {
    // End connection
    // Call twice
    int ret = SSL_shutdown(ssl);
    if (ret == 0) {
        SSL_shutdown(ssl);
    }
}

SSL_CTX *create_openvpn_ctx(bool server, char *privkey, char *cert) {
    SSL_METHOD *method = server ? TLS_server_method() : TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        printf("Failed to create OpenSSL context");
    }

    // Options
    uint64_t openssl_flags =
        SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(ctx, openssl_flags);

    if (cert != NULL) {
        if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
            printf("Failed to set certificate\n");
        }
    }

    if (privkey != NULL) {
        if (SSL_CTX_use_PrivateKey_file(ctx, privkey, SSL_FILETYPE_PEM) != 1) {
            printf("Failed to set privkey\n");
        }
    }

    return ctx;
}

SSL *create_openvpn_ssl(SSL_CTX *ctx) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL) {
        printf("Failed to create SSL");
    }

    // Options
    SSL_set1_groups_list(ssl, "X25519MLKEM768:X25519");

    return ssl;
}

void openvpn_client_start(struct openvpn_client_config config) {
    struct openvpn_client_state state;

    struct sockaddr_in address = config.server_address;
    int socket_type = config.socket_type;

    int sock = socket(address.sin_family, socket_type, 0);
    if (sock < 0) {
        perror("Failed to open socket");
        return;
    }

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Failed to connect to server");
        return;
    }

    SSL_CTX *ctx = create_openvpn_ctx(false, NULL, NULL);
    SSL *ssl = create_openvpn_ssl(ctx);

    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);

    if (init_tun_tap_device(&state.tt) < 0) {
        return;
    };

    printf("sizeof(ssize_t) = %u", sizeof(ssize_t));

    while (1) {
        char buf[1024];
        memset(buf, '\0', sizeof(buf));

        ssize_t len = read(state.tt.fd, buf, sizeof(buf) - 1);
        printf("Read %lu bytes from tun: %s\n", len, buf);

        SSL_write(ssl, buf, len);
    }

    end_connection(ssl);

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
}

void openvpn_server_start(struct openvpn_server_config config) {
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.port);

    int socket_type = config.socket_type;

    int sock = socket(address.sin_family, socket_type, 0);
    // connect(sock, (struct sockaddr *)&address, sizeof(address));
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) != 0) {
        printf("Failed to bind TCP port %i\n", config.port);
        exit(1);
    }

    if (listen(sock, 10) != 0) {
        printf("Failed to listen to TCP socket");
        exit(1);
    }

    SSL_CTX *ctx = create_openvpn_ctx(true, config.privkey, config.certificate);

    while (1) {

        struct sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);

        int con_sock = accept(sock, (struct sockaddr *)&client_addr, &addrlen);
        SSL *ssl = create_openvpn_ssl(ctx);

        if (SSL_set_fd(ssl, con_sock) == 0) {
            printf("Failed to set SSL fd");
        }
        pthread_t tid;
        pthread_create(&tid, NULL, &openvpn_server_handle_client, (void *)ssl);
        printf("Started thread with thread ID %lu\n", tid);
    }

    SSL_CTX_free(ctx);
}

void *openvpn_server_handle_client(void *entry) {

    printf("Accepting Client\n");

    SSL *ssl = (SSL *)entry;
    int conn_fd = SSL_get_fd(ssl);

    int ret = SSL_accept(ssl);
    if (ret <= 0) {
        printf("Failed to accept SSL connection!\n");
        // ERR_print_errors_fp(stdout);
        int error = SSL_get_error(ssl, ret);
        if (error != SSL_ERROR_SSL) {
            end_connection(ssl);
        }

        SSL_free(ssl);
        close(conn_fd);

        pthread_exit(NULL);
    }

    char buf[1024];
    while (1) {
        memset(buf, '\0', sizeof(buf));

        int len = SSL_read(ssl, buf, sizeof(buf));

        if (len <= 0) {
            break;
        }

        if (strncmp(buf, "exit\n", 5) == 0) {
            break;
        }

        printf("Recieved %u bytes:\n",len);
        print_bytes(buf, len);
        printf("\n");
    }

    printf("Disconnecting from Client\n");

    end_connection(ssl);
    SSL_free(ssl);
    close(conn_fd);

    return NULL;
}