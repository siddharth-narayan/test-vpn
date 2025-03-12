#include "network.h"

#include <stdio.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "src/util/print.h"

#ifdef __linux__
#include <linux/filter.h>
#include <asm-generic/socket.h>
#include <netinet/ip.h>

#include "src/os/linux/netlink.h"
#endif

#ifdef __linux
raw_socket sock_ipv4 = -1;
raw_socket sock_ipv6 = -1;
#endif

SSL *create_ssl(SSL_CTX *ctx) {
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL) {
        printf("Failed to create SSL");
    }

    // Options
    SSL_set1_groups_list(ssl, "X25519MLKEM768:X25519");

    return ssl;
}

SSL_CTX *create_ctx(bool server, char *privkey, char *cert) {
    SSL_METHOD *method = server ? TLS_server_method() : TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        printf("Failed to create OpenSSL context");
    }

    // Options
    uint64_t openssl_flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
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

void end_connection(SSL *ssl) {
    // End connection
    // Call twice
    int ret = SSL_shutdown(ssl);
    if (ret == 0) {
        SSL_shutdown(ssl);
    }
}

raw_socket frame_socket_with_filter(struct sock_fprog filter) {
#ifdef __linux__
    int fd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

    if (fd < 0) {
        perror("Failed to create socket");
        return fd;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(struct sock_fprog)) < 0) {
        perror("Failed to set socket filter");
    }

    return fd;
#endif
}

void forward_packet_raw(uint8_t *buf, int len) {
#ifdef __linux__
    uint16_t family = buf[2] << 8 | buf[3];
    if (family == 0x0800) {
        printf("Family is AF_INET\n");
        forward_packet_ipv4(buf + 4, len - 4);
    }

    if (family == 0x86DD) {
        printf("Family is AF_INET6\n");
        forward_packet_ipv6(buf + 4, len - 4);
    }
#endif
}

void forward_packet_ipv4(uint8_t *buf, int len) {
    if (sock_ipv4 < 0) {
        sock_ipv4 = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sock_ipv4 < 0) {
            perror("Failed to create socket");
        }

        int opt = 1;
        setsockopt(sock_ipv4, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt));
    }

    struct iphdr *header = (struct iphdr *)buf;
    struct sockaddr_in destination;

    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = header->daddr;

    if (sendto(sock_ipv4, buf, len, 0, (struct sockaddr *)&destination, sizeof(destination)) < 0) {
        perror("Failed to send packet to IPv4 destination");
    }
}

void forward_packet_ipv6(uint8_t *buf, int len) {}

int init_tun_tap_device(struct tun_tap_device *tt) {
#ifdef __linux__
    int netlink_fd = netlink_connect();

    if (linux_tun_tap_initialize(tt, true, 0) < 0) {
        return -1;
    }

    netlink_set_tun_tap_addr_ipv4(netlink_fd, tt->if_index, "72.100.100.100", 24);

    if (tun_tap_add_flag(tt, IFF_UP) < 0) {
        return -1;
    }

    close(netlink_fd);
    return 0;
#endif
}

int tun_tap_write(struct tun_tap_device *tt, uint8_t *buf, int len) {
#ifdef __linux__
    if (tt->fd > 0) {
        return write(tt->fd, buf, len);
    }

    return -1;
#endif
}

int tun_tap_read(struct tun_tap_device *tt, uint8_t *buf, int len) {
#ifdef __linux__
    if (tt->fd > 0) {
        return read(tt->fd, buf, len);
    }

    return -1;
#endif
}