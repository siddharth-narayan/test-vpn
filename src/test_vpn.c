#include <stdio.h>

#include "protocols/openvpn/openvpn.h"

#define PROJECT_NAME "test-vpn"

int main(int argc, char **argv) {
    if(argc != 1) {
        printf("%s takes no arguments.\n", argv[0]);
        return 1;
    }
    printf("This iswfwf project %s.\n", PROJECT_NAME);
    printhaha();
    return 0;
}
