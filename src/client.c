#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdio.h>

#include "protocols/tvpn/tvpn.h"
#include "util/args.h"

int main(int argc, char **argv) {
    args args = { argc, argv };

    SSL_library_init();
    SSL_load_error_strings();
    
    struct tvpn_c_config conf;

    inet_aton(argv[arg_parse_str(args, "-a")], &conf.server_address.sin_addr); // Address
    arg_parse_uint(args, "-p", conf.server_address.sin_port); // Port
    conf.server_address.sin_port = htons(conf.server_address.sin_port);  
    
    conf.server_address.sin_family = AF_INET;
    conf.socket_type = SOCK_STREAM;

    tvpn_c_start(conf);
    return 0;
}
