#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdio.h>

#include "protocols/openvpn/openvpn.h"

int main(int argc, char **argv) {
    SSL_library_init();
    SSL_load_error_strings();
    
    struct openvpn_client_config conf;
    inet_aton("127.0.0.1", &conf.server_address.sin_addr);
    conf.server_address.sin_family = AF_INET;
    conf.server_address.sin_port = htons(1194);  
    conf.socket_type = SOCK_STREAM;

    openvpn_client_start(conf);
    return 0;
}
