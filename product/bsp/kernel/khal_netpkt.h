#ifndef __KHAL_NETPKT_H__
#define __KHAL_NETPKT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "kbsp_types.h"

#define NETPKT_TOCPU        1
#define NETPKT_FROMDEV      2
#define NETPKT_TOSWITCH     3
#define NETPKT_FROMSWITCH   4

#define NETPKT_ETHERNET_HEADER   14
#define NETPKT_ETHMAC_HEADER   12
typedef struct
{
    zpl_uint32      cmd;                         /* Unit number. */
    zpl_phyport_t   dstval;          

}khal_nettpkt_cmd_t __attribute__ ((packed));




int khal_netpkt_init(void);
void khal_netpkt_exit(void);
int khal_netpkt_debug_set(int set, int val);
int khal_netpkt_bind(int ifindex, int bind);
int khal_netpkt_dstpid(int pid);
void khal_netpkt_fromdev(struct net_device *dev, struct sk_buff *skb);

#ifdef __cplusplus
}
#endif

#endif /* __KHAL_NETPKT_H__ */
