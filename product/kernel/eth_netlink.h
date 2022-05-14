#ifndef __NET_LINK_H__
#define __NET_LINK_H__

#define NETLINK_TEST (25)

typedef struct eth_netlink_s
{
    dev_t devId;
    struct class *cls;
    struct sock *nl_sk;
}eth_netlink_t;


int eth_netlink_init(void);
void eth_netlink_exit(void);
int eth_send_userspace(int msgtype, uint8_t *message, int len);
int eth_from_userspace(int msgtype, uint8_t *message, int len);

#endif /* __NET_LINK_H__ */