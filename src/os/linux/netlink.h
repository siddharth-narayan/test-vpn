#pragma once

#include "src/network/network.h"
#include "stdbool.h"
#include "stdint.h"

int netlink_set_tun_tap_addr_ipv4(int netlink_fd, int ifindex, const char *address,
                              uint8_t network_prefix_bits);

// Connects to netlink and returns the file descriptor for communication
int netlink_connect();

int tun_tap_add_flag(struct tun_tap_device *tt, uint16_t flag);

// Creates a default TUN device with a default name and returns *something*
int linux_tun_tap_initialize(struct tun_tap_device *tt, bool tun, short flags);