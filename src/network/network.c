#include "network.h"
#include "src/util/print.h"
#ifdef __linux__
#include "src/os/linux/netlink.h"
#include <netinet/ip.h>
#endif

#include <stdio.h>
#include <unistd.h>

#ifdef __linux
int sock_ipv4 = -1;
int sock_ipv6 = -1;
#endif

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

void forward_packet_ipv6(uint8_t *buf, int len) {
    
}


int init_tun_tap_device(struct tun_tap_device *tt) {
#ifdef __linux__
    int netlink_fd = netlink_connect();

    if (linux_tun_tap_initialize(tt, true, 0) < 0) {
        return -1;
    }

    netlink_set_tun_tap_addr_ipv4(netlink_fd, tt->if_index, "72.100.100.100",
                                  24);

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