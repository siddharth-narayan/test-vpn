#include "tvpn.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <unistd.h>

#include "src/network/network.h"
#include "src/util/print.h"

void tvpn_c_start(struct tvpn_c_config config) {
    struct tvpn_c_state state;

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

    SSL_CTX *ctx = create_ctx(false, NULL, NULL);
    SSL *ssl = create_ssl(ctx);

    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);

    if (init_tun_tap_device(&state.tt) < 0) {
        return;
    };

    while (1) {
        char buf[1024];
        memset(buf, '\0', sizeof(buf));

        ssize_t len = read(state.tt.fd, buf, sizeof(buf) - 1);
        printf("Sending %lu bytes from tun\n", len);

        SSL_write(ssl, buf, len);
    }

    end_connection(ssl);

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
}

void tvpn_s_start(struct tvpn_s_config config) {
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

    SSL_CTX *ctx = create_ctx(true, config.privkey, config.certificate);

    while (1) {

        struct sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);

        int con_sock = accept(sock, (struct sockaddr *)&client_addr, &addrlen);

        SSL *ssl = create_ssl(ctx);

        if (SSL_set_fd(ssl, con_sock) == 0) {
            printf("Failed to set SSL fd");
        }

        printf("Accepting Client\n");
        int ret = SSL_accept(ssl);
        if (ret <= 0) {
            printf("Failed to accept SSL connection!\n");
            // ERR_print_errors_fp(stdout);
            int error = SSL_get_error(ssl, ret);
            if (error != SSL_ERROR_SSL) {
                end_connection(ssl);
            }

            SSL_free(ssl);
            close(con_sock);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, &tvpn_s_handle_client, (void *)ssl);
        printf("Started thread with thread ID %lu\n", tid);
    }

    SSL_CTX_free(ctx);
}

void *tvpn_s_handle_client(void *entry) {
    struct tvpn_client *client;
    client->ssl = (SSL *)entry;
    int conn_fd = SSL_get_fd(client->ssl);

    pthread_t recv;
    pthread_t write;
    pthread_create(&recv, NULL, &tvpn_s_recv, (void *)client);
    pthread_create(&write, NULL, &tvpn_s_write, (void *)client);
    pthread_join(recv, NULL);
    pthread_join(write, NULL);

    printf("Disconnecting from Client\n");

    end_connection(client->ssl);
    SSL_free(client->ssl);

    close(conn_fd);

    return NULL;
}

void *tvpn_s_recv(void *entry) {
    struct tvpn_client *client = entry;
    char buf[1024];
    while (1) {
        memset(buf, '\0', sizeof(buf));

        int len = SSL_read(client->ssl, buf, sizeof(buf));

        if (len <= 0) {
            return NULL;
        }

        printf("Recieved %u bytes:\n", len);
        print_bytes(buf, len);
        printf("\n");

        forward_packet_raw((uint8_t *)buf, len);
    }
}

void *tvpn_s_write(void *entry) {
    struct tvpn_client *client = entry;

    struct sock_filter code[] = {
        {0x28, 0, 0, 0x0000000c}, {0x15, 0, 5, 0x00000800}, {0x20, 0, 0, 0x0000001a}, {0x15, 2, 0, 0x48646464},
        {0x20, 0, 0, 0x0000001e}, {0x15, 0, 1, 0x48646464}, {0x6, 0, 0, 0x00040000},  {0x6, 0, 0, 0x00000000},
    };

    struct sock_fprog filter = {8, code};

    int fd = frame_socket_with_filter(filter);

    char buf[1024];
    while (1) {
        memset(buf, '\0', sizeof(buf));

        int len = recv(fd, buf, sizeof(buf), 0);

        if (len <= 0) {
            return NULL;
        }

        printf("Recieved %u bytes destined for 72.100.100.100:\n", len);
        print_bytes(buf, len);
        printf("\n");
    }
}
