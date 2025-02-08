#include "stdint.h"

int netlink_set_addr_ipv4(int netlink_fd, int ifindex,
                          const char *address, uint8_t network_prefix_bits);

int netlink_connect();

// Creates a TUN device with a default name and returns its ifindex
int tuntap_add_interface(short flags);