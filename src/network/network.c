#include "network.h"

#ifdef __linux__
#include <unistd.h>
#include "src/os/linux/netlink.h"
#endif

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