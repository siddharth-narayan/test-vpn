#include "network.h"

#ifdef __linux__
#include "src/os/linux/netlink.h"
#endif

void add_tun_device() {
#ifdef __linux__
    int netlink_fd = netlink_connect();
    int new_ifindex = tuntap_add_interface(0);
    netlink_set_addr_ipv4(netlink_fd, new_ifindex, "72.100.100.100", 24);
#endif
}