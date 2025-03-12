#pragma once

#include <stdbool.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <linux/filter.h>
#include <linux/if.h>

SSL *create_ssl(SSL_CTX *ctx);
SSL_CTX *create_ctx(bool server, char *privkey, char *cert);

void end_connection(SSL *ssl);


#ifdef __linux__
typedef int raw_socket;
#endif

raw_socket frame_socket_with_filter(struct sock_fprog filter);
void forward_packet_raw(uint8_t *buf, int len);
void forward_packet_ipv4(uint8_t *buf, int len);
void forward_packet_ipv6(uint8_t *buf, int len);

struct tun_tap_device {
#ifdef __linux__
    bool tun;
    int fd;
    int if_index;
    char name[IFNAMSIZ];
#endif
};

int init_tun_tap_device(struct tun_tap_device *tt);

int tun_tap_read(struct tun_tap_device *tt, uint8_t *buf, int len);
int tun_tap_write(struct tun_tap_device *tt, uint8_t *buf, int len);