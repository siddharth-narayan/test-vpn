#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/netlink.h>

int netlink_set_addr_ipv4(int netlink_fd, int ifindex,
                          const char *address, uint8_t network_prefix_bits) {
    struct {
        struct nlmsghdr header;
        struct ifaddrmsg content;
        char attributes_buf[64];
    } request;

    printf("index %u\n", if_nametoindex("tun0"));
    struct rtattr *request_attr;
    size_t attributes_buf_avail = sizeof request.attributes_buf;

    memset(&request, 0, sizeof(request));
    request.header.nlmsg_len = NLMSG_LENGTH(sizeof request.content);
    request.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | NLM_F_CREATE;
    request.header.nlmsg_type = RTM_NEWADDR;
    request.content.ifa_index = ifindex;
    request.content.ifa_family = AF_INET;
    request.content.ifa_prefixlen = network_prefix_bits;

    /* request.attributes[IFA_LOCAL] = address */
    request_attr = IFA_RTA(&request.content);
    request_attr->rta_type = IFA_LOCAL;
    request_attr->rta_len = RTA_LENGTH(sizeof(struct in_addr));

    request.header.nlmsg_len += request_attr->rta_len;
    inet_pton(AF_INET, address, RTA_DATA(request_attr));

    /* request.attributes[IFA_ADDRESS] = address */
    request_attr = RTA_NEXT(request_attr, attributes_buf_avail);
    request_attr->rta_type = IFA_ADDRESS;
    request_attr->rta_len = RTA_LENGTH(sizeof(struct in_addr));

    request.header.nlmsg_len += request_attr->rta_len;
    inet_pton(AF_INET, address, RTA_DATA(request_attr));

    if (send(netlink_fd, &request, request.header.nlmsg_len, 0) == -1) {
        return -1;
    }
    
    return 0;
}

int tuntap_add_interface(short flags) {
    int tuntap_fd, ret;
    struct ifreq setiff_request;

    tuntap_fd = open("/dev/net/tun", O_RDWR);
    if (tuntap_fd == -1) {
        return -1;
    }

    memset(&setiff_request, 0, sizeof(setiff_request));
    setiff_request.ifr_flags = flags | IFF_TUN;

    ret = ioctl(tuntap_fd, TUNSETIFF, &setiff_request);

    if (ret == -1) {
        close(tuntap_fd);
        return -1;
    }

    printf("TUN device created with name %s\n", setiff_request.ifr_name);
    return setiff_request.ifr_ifindex;
}

int netlink_connect() {
    int netlink_fd, rc;
    struct sockaddr_nl sockaddr;

    netlink_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (netlink_fd == -1) {
        return -1;
    }

    memset(&sockaddr, 0, sizeof sockaddr);
    sockaddr.nl_family = AF_NETLINK;
    rc = bind(netlink_fd, (struct sockaddr *)&sockaddr, sizeof sockaddr);
    if (rc == -1) {
        int bind_errno = errno;
        close(netlink_fd);
        errno = bind_errno;
        return -1;
    }
    return netlink_fd;
}