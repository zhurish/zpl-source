#ifndef __KHAL_NETLINK_H__
#define __KHAL_NETLINK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "kbsp_types.h"
#include "khal_ipccmd.h"
#include "khal_ipcmsg.h"

#define HAL_CFG_REQUEST_CMD (30) 
#define HAL_DATA_REQUEST_CMD (29)
#define HAL_KLOG_REQUEST_CMD (28)

#define HAL_CFG_NETLINK_PROTO (30)
#define HAL_DATA_NETLINK_PROTO (29)
#define HAL_KLOG_NETLINK_PROTO (28)

struct khal_netlink
{
    struct sock *nlsock;
    zpl_uint32 proto;
    char    name[16];
    zpl_uint32 debug;
    zpl_uint32 cmd;
    zpl_uint32 seqno;
    zpl_uint32 dstpid;
    zpl_uint32 group;   
};

struct khal_netlink *khal_netlink_create(char *name, int proto, int group, struct netlink_kernel_cfg *cfg);
void khal_netlink_destroy(struct khal_netlink *khal_netlink);

void khal_netlink_group_dstpid(struct khal_netlink *khal_netlink, zpl_uint32 group, zpl_uint32 pid);
int khal_netlink_unicast(struct khal_netlink *hal_client, zpl_uint32 pid, struct sk_buff *skb);
int khal_netlink_multicast(struct khal_netlink *hal_client, zpl_uint32 group, struct sk_buff *skb);


#ifdef __cplusplus
}
#endif

#endif /* __KHAL_NETLINK_H__ */
