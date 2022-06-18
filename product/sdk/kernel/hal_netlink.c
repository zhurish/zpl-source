#include "bsp_types.h"
#include "hal_netlink.h"
#include "bsp_include.h"



struct hal_netlink *hal_netlink_create(char *name, int proto, int group, struct netlink_kernel_cfg *cfg)
{
  struct hal_netlink *hal_netlink = XMALLOC(MTYPE_HALIPCSRV, sizeof(struct hal_netlink));
  if(hal_netlink)
  {
    	//初始化netlink
      memset(hal_netlink, 0, sizeof(struct hal_netlink));
	    hal_netlink->nlsock = netlink_kernel_create(&init_net, proto, cfg);
	    if (!hal_netlink->nlsock) 
        {
		    zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
            XFREE(MTYPE_HALIPCSRV, hal_netlink);
            hal_netlink = NULL;
            return hal_netlink;
	    }
        strcpy(hal_netlink->name, name);
        hal_netlink->group = group;
  }
  return hal_netlink;
}

void hal_netlink_destroy(struct hal_netlink *hal_netlink)
{
  if(hal_netlink)
  {
    if(hal_netlink->nlsock)
    {
        netlink_kernel_release(hal_netlink->nlsock);
        hal_netlink->nlsock = NULL;
    }
    XFREE(MTYPE_HALIPCSRV, hal_netlink);
    hal_netlink = NULL;
  }
}

void hal_netlink_group_dstpid(struct hal_netlink *hal_netlink, zpl_uint32 group, zpl_uint32 pid)
{
  hal_netlink->group = group;
  hal_netlink->dstpid = pid;
}

int hal_netlink_unicast(struct hal_netlink *hal_netlink, zpl_uint32 pid, struct sk_buff *skb)
{
	if(skb && hal_netlink->nlsock)
	{
		return netlink_unicast(hal_netlink->nlsock, (struct sk_buff *)skb, pid, MSG_DONTWAIT);
	}
	return 0;  
}


int hal_netlink_multicast(struct hal_netlink *hal_netlink, zpl_uint32 group, struct sk_buff *skb)
{
	if(skb && hal_netlink->nlsock)
	{
        NETLINK_CB(skb).dst_group = group;
		return netlink_broadcast(hal_netlink->nlsock, (struct sk_buff *)skb, 0, group, MSG_DONTWAIT);
	}
	return 0;  
}
	