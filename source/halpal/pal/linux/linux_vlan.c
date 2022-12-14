/*
 * ipkernel_vlaneth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "vrf.h"
#include "pal_include.h"
#include "nsm_debug.h"
#include "nsm_vlan.h"
#include "nsm_arp.h"
#include "nsm_bridge.h"
#include "nsm_firewalld.h"
#include "nsm_vlaneth.h"
#include "linux_driver.h"
#define _LINUX_IP_H
#include <linux/if_tun.h>
#include "linux/if_vlan.h"
#include "linux/sockios.h"


#ifdef ZPL_KERNEL_NETLINK


static int ip_vlan_create(const char *name, vlan_t vid)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = ADD_VLAN_CMD;
	strcpy(v_req.device1,name);
	v_req.u.VID = vid;
	ret = linux_ioctl_if_ioctl (IPSTACK_SIOCSIFVLAN, &v_req);
	return ret;
}

static int ip_vlan_delete(const char *name, vlan_t vid)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = DEL_VLAN_CMD;
	strcpy(v_req.device1,name);
	v_req.u.VID = vid;
	ret = linux_ioctl_if_ioctl (IPSTACK_SIOCSIFVLAN, &v_req);
	//ret = ipstack_ioctl(vlan_sock_fd, IPSTACK_SIOCSIFVLAN, &v_req);
	return ret;
}

#if 0
static int ip_vlan_name_type(zpl_uint32 type)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = SET_VLAN_NAME_TYPE_CMD;
	v_req.u.name_type = type;
	ret = linux_ioctl_if_ioctl(SIOCSIFVLAN, &v_req);
	return ret;
}


static int ip_vlan_egress(const char *vname, int skb_priority, int vlan_qos)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = SET_VLAN_EGRESS_PRIORITY_CMD;
	strcpy(v_req.device1,vname);
	v_req.u.skb_priority = skb_priority;
	v_req.vlan_qos = vlan_qos;
	ret = linux_ioctl_if_ioctl(SIOCSIFVLAN, &v_req);
	return ret;
}

static int ip_vlan_ingress(const char *vname, int skb_priority, int vlan_qos)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = SET_VLAN_INGRESS_PRIORITY_CMD;
	strcpy(v_req.device1,vname);
	v_req.u.skb_priority = skb_priority;
	v_req.vlan_qos = vlan_qos;
	ret = linux_ioctl_if_ioctl(SIOCSIFVLAN, &v_req);
	return ret;
}
#endif

int linux_ioctl_vlan_create (struct interface *ifp)
{
	nsm_vlaneth_t *kifp = nsm_vlaneth_get(ifp);
	if(kifp && ip_vlan_create(kifp->root->ker_name, kifp->vlanid) == 0)
	{
		return OK;
	}
	return ERROR;
}

int linux_ioctl_vlan_destroy (struct interface *ifp)
{
	nsm_vlaneth_t *kifp = nsm_vlaneth_get(ifp);
	if(kifp)
		return ip_vlan_delete(kifp->root->ker_name, kifp->vlanid);
	return ERROR;	
}


int linux_ioctl_vlan_change (struct interface *ifp, vlan_t vlan)
{
	nsm_vlaneth_t *kifp = nsm_vlaneth_get(ifp);
	if(kifp && if_is_ethernet(kifp->ifp) && kifp->root && IF_ID_GET(kifp->ifp->ifindex))
	{
		if(kifp->ifp->ll_type == IF_LLT_MODEM)
			return OK;
		if(kifp->active)
			linux_ioctl_vlan_destroy(kifp);
		kifp->vlanid = vlan;
		if(linux_ioctl_vlan_create(kifp) == 0)
		{
			kifp->active = zpl_true;
			return OK;
		}
	}
	return ERROR;
}

#endif