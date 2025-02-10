#include <stdio.h>

#include "print.h"

const int WIDTH = 20;
void print_bytes(char *buf, int len) {
    // Wrap every 80 bytes
    int i = 0;
    while (i < len / WIDTH) {
        for (int j = 0; j < WIDTH; j++) {
            printf("%02hhx ", buf[i * WIDTH + j]);
        }

        printf("\n");

        i++;
    }

    // Print the remaining bytes
    for (int j = 0; j < len % WIDTH; j++) {
        printf("%02hhx ", buf[i * WIDTH + j]);
    }

    printf("\n");
}