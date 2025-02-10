#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <linux/if.h>

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