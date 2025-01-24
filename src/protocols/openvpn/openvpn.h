#include <stdint.h>
#include <sys/socket.h>

struct openvpn_config {
    struct sockaddr addr;
    
}

struct openvpn_state {
    uint8_t state[1000];
};

struct openvpn_packet {
    uint16_t packet_length;
    union optional_key
    {
        struct key
        {
            uint8_t key_id;
            uint8_t data[999];
        } key;
        
        uint8_t data[1000];
         
    } optional_key;
}

enum openvpn_message_type {
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

void printhaha();