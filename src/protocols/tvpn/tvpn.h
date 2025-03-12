#pragma once
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#include "src/network/network.h"

// A struct representing a client on the server side
struct tvpn_client {
    SSL *ssl;
    bool ipv4;
    union {
        struct in_addr addr_v4;
        struct in6_addr addr_v6;
    } address;
};

struct tvpn_s_state {
    uint8_t state[1000];
};

struct tvpn_c_state {
    struct tun_tap_device tt;
    uint8_t state[1000];
};

struct tvpn_c_config {
    struct sockaddr_in server_address;
    enum __socket_type socket_type; // TCP or UDP?
};

struct tvpn_s_config {
    uint16_t port;
    char *privkey;
    char *certificate;
    enum __socket_type socket_type; // TCP or UDP?
};

void tvpn_c_start(struct tvpn_c_config);
void tvpn_s_start(struct tvpn_s_config);

void *tvpn_s_handle_client(void *);
void *tvpn_s_recv(void *); // Responsible from reading from the client and forwarding packets
void *tvpn_s_write(void *); // Responsible from reading from the server and forwarding to the client