#ifndef __KHAL_NETPKT_H__
#define __KHAL_NETPKT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_types.h"

#define NETPKT_TOCPU        1
#define NETPKT_FROMDEV      2
#define NETPKT_TOSWITCH     3
#define NETPKT_FROMSWITCH   4

typedef struct
{
    zpl_uint8       cmd;                         /* Unit number. */
    zpl_phyport_t   dstval;          

}zpl_netpkt_cmd_t __attribute__ ((aligned (1)));




int netpkt_netlink_init(void);
void netpkt_netlink_exit(void);
int netpkt_netlink_debug_set(int set, int val);
int netpkt_netlink_bind(int ifindex, int bind);
int netpkt_netlink_dstpid(int pid);
void netpkt_fromdev(struct net_device *dev, struct sk_buff *skb);

#ifdef __cplusplus
}
#endif

#endif /* __KHAL_NETPKT_H__ */
