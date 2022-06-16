#include "auto_include.h"
#include "zplos_include.h"
#include "zebra_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_debug.h"
#include "nsm_rib.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "nsm_include.h"
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

#include "linux_driver.h"
#include "pal_include.h"
//#include "bsp_include.h"

#include "linux_ethlink.h"

#define NETLINK_TEST (25)
#define MAX_PAYLOAD (1024)



static int netlink_bind(int sock_fd)
{
    struct sockaddr_nl addr;

    memset(&addr, 0, sizeof(struct sockaddr_nl));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0;
    return bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_nl));
}

int netlink_create_socket(void)
{
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if(sock)
    {
        netlink_bind(sock);
    }
    return sock;
}

static int netlink_send_message(int sock_fd, int msgtype, const unsigned char *message, int len, int seq)
{
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl dest_addr;
    struct iovec iov;
    struct msghdr msg;

    if (!message)
    {
        return -1;
    }
    // create message
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
    if (!nlh)
    {
        perror("malloc");
        return -2;
    }
    nlh->nlmsg_len = NLMSG_SPACE(len);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = msgtype;
    nlh->nlmsg_seq = seq;
    memcpy(NLMSG_DATA(nlh), message, len);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    
    memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = getpid();
    dest_addr.nl_groups = 0;

    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    // send message
    if (sendmsg(sock_fd, &msg, 0) < 0)
    {
        printf("send error!\n");
        free(nlh);
        return -3;
    }
    free(nlh);
    return 0;
}

static int netlink_recv_message(int sock_fd, int (*func)(int msgtype, char *msg, int len))
{
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl source_addr;
    struct iovec iov;
    struct msghdr msg;

    if (!func)
    {
        return -1;
    }

    // create message
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh)
    {
        perror("malloc");
        return -2;
    }
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
    memset(&source_addr, 0, sizeof(struct sockaddr_nl));
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_name = (void *)&source_addr;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (recvmsg(sock_fd, &msg, 0) < 0)
    {
        printf("recvmsg error!\n");
        return -3;
    }
    //*len = nlh->nlmsg_len - NLMSG_SPACE(0);
    // memcpy(message, (unsigned char *)NLMSG_DATA(nlh), *len);
    (func)(nlh->nlmsg_type, (unsigned char *)NLMSG_DATA(nlh), nlh->nlmsg_len - NLMSG_SPACE(0));
    free(nlh);
    return 0;
}

int netlink_message_request(struct netlink_sock *nlsock, int msgtype, unsigned char *message, int len)
{
    if (netlink_send_message(nlsock->sock._fd, msgtype, message, len, nlsock->seq) == 0)
        return netlink_recv_message(nlsock->sock._fd, nlsock->netlink_func);
    return ERROR;
}


/*
int
main(int argc, char **argv)
{
        int sock_fd;
        char buf[MAX_PAYLOAD];
        int len;

        if( argc < 2) {
                printf("enter message!\n");
                exit(EXIT_FAILURE);
        }

        sock_fd = netlink_create_socket();
        if(sock_fd == -1) {
                printf("socket error!\n");
                return -1;
        }

        if( netlink_bind(sock_fd) < 0 ) {
                perror("bind");
                close(sock_fd);
                exit(EXIT_FAILURE);
        }

        netlink_send_message(sock_fd, argv[1], strlen(argv[1]) + 1, 0, 0);
        if( netlink_recv_message(sock_fd, buf, &len) == 0 ) {
                printf("recv:%s len:%d\n", buf, len);
        }

        close(sock_fd);
        return 0;
}
*/