#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <signal.h>

#include "protocols/openvpn/openvpn.h"


int main(int argc, char **argv) {
    SSL_library_init();
    SSL_load_error_strings();

    signal(SIGPIPE, SIG_IGN);

    struct openvpn_server_config conf;
    // inet_aton("96.252.17.110", &conf.server_address.sin_addr);
    // conf.server_address.sin_family = AF_INET;
    conf.port = 1194;  
    conf.socket_type = SOCK_STREAM;
    conf.privkey = "key.pem";
    conf.certificate = "cert.pem";
    
    openvpn_server_start(conf);
    return 0;
}
