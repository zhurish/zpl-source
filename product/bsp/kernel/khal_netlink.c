#include "kbsp_types.h"
#include "khal_netlink.h"
#ifdef ZPL_SDK_KERNEL
#include "bsp_include.h"
#endif


struct khal_netlink *khal_netlink_create(char *name, int proto, int group, struct netlink_kernel_cfg *cfg)
{
  struct khal_netlink *khal_netlink = XMALLOC(MTYPE_HALIPCSRV, sizeof(struct khal_netlink));
  if(khal_netlink)
  {
    	//初始化netlink
      memset(khal_netlink, 0, sizeof(struct khal_netlink));
	    khal_netlink->nlsock = netlink_kernel_create(&init_net, proto, cfg);
	    if (!khal_netlink->nlsock) 
        {
		    zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
            XFREE(MTYPE_HALIPCSRV, khal_netlink);
            khal_netlink = NULL;
            return khal_netlink;
	    }
        strcpy(khal_netlink->name, name);
        khal_netlink->group = group;
  }
  return khal_netlink;
}

void khal_netlink_destroy(struct khal_netlink *khal_netlink)
{
  if(khal_netlink)
  {
    if(khal_netlink->nlsock)
    {
        netlink_kernel_release(khal_netlink->nlsock);
        khal_netlink->nlsock = NULL;
    }
    XFREE(MTYPE_HALIPCSRV, khal_netlink);
    khal_netlink = NULL;
  }
}

void khal_netlink_group_dstpid(struct khal_netlink *khal_netlink, zpl_uint32 group, zpl_uint32 pid)
{
  khal_netlink->group = group;
  khal_netlink->dstpid = pid;
}

int khal_netlink_unicast(struct khal_netlink *khal_netlink, zpl_uint32 pid, struct sk_buff *skb)
{
	if(skb && khal_netlink->nlsock)
	{
		return netlink_unicast(khal_netlink->nlsock, (struct sk_buff *)skb, pid, MSG_DONTWAIT);
	}
	return 0;  
}


int khal_netlink_multicast(struct khal_netlink *khal_netlink, zpl_uint32 group, struct sk_buff *skb)
{
	if(skb && khal_netlink->nlsock)
	{
    NETLINK_CB(skb).dst_group = group;
		return netlink_broadcast(khal_netlink->nlsock, (struct sk_buff *)skb, 0, group, MSG_DONTWAIT);
	}
	return 0;  
}
	