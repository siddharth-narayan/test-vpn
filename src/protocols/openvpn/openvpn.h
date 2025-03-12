#pragma once

#include <stdint.h>

struct openvpn_server_state {
    uint8_t state[1000];
};

struct openvpn_state {
    struct tun_tap_device tt;
    uint8_t state[1000];
};

struct openvpn_packet {
    uint16_t packet_length;
    union optional_key {
        struct key {
            uint8_t key_id;
            uint8_t data[999];
        } key;

        uint8_t data[1000];

    } optional_key;
};

enum tvpn_message_type {
    UNUSED0,
    UNUSED1,
    UNUSED2,
    P_CONTROL_HARD_RESET_CLIENT_V1,
    P_CONTROL_HARD_RESET_SERVER_V1,
    P_CONTROL_SOFT_RESET_V1,
    P_CONTROL_V1,
    P_ACK_V1,
    P_DATA_V1,
    P_CONTROL_HARD_RESET_CLIENT_V2,
    P_CONTROL_HARD_RESET_SERVER_V2,
};