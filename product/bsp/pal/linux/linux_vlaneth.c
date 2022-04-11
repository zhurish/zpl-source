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

#ifdef ZPL_NSM_VLANETH

#define IPKERNEL_TUN_NAME	"/dev/net/tun"
#define IPKERNEL_TAP_NAME	"/dev/tap"

static int _linux_kernel_vlaneth_create (nsm_vlaneth_t *kifp)
{
	struct ipstack_ifreq ifr;
	struct interface *ifp;
    zpl_uchar macadd[6] = {0x20,0x5f,0x87,0x65,0x66,0x00};
	if (!kifp || !kifp->ifp)
		return -1;
	ifp = kifp->ifp;
	memset(&ifr, 0, sizeof(ifr));
	/*
	 *if we are using the tap driver here
	 *for the dtl interface then we need to
	 *a file descriptor to it and set up
	 *the corresponding network interface
	 */
	if(if_is_serial(ifp))
	{
		kifp->fd = ipstack_open(IPCOM_STACK, IPKERNEL_TUN_NAME, O_RDWR);
		if (ipstack_invalid(kifp->fd))
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TUN_NAME, ipstack_strerror(ipstack_errno));
			return ERROR;
		}
	}
	else
	{
		kifp->fd = ipstack_open(IPCOM_STACK, IPKERNEL_TAP_NAME, O_RDWR);
		if (ipstack_invalid(kifp->fd))
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TUN_NAME, ipstack_strerror(ipstack_errno));
			return ERROR;
		}
	}
	/*
	 *register this fd with the tap monitor.
	 *make sure to add a dtlCmd function
	 */
	if(if_is_serial(ifp))
	{
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	}
	else
	{
		ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	}
	strcpy(ifr.ifr_name, ifp->k_name);
	/*
	 *Inidicate that this is a tap interface
	 *and that we provide no additional packet information
	 */

	if (ipstack_ioctl(kifp->fd, IPSTACK_TUNSETIFF, &ifr) < 0)
	{
		zlog_err(MODULE_PAL, "Unable to create corresponding L3 interface %s(%s).",
				ifp->name, ipstack_strerror(ipstack_errno));
		ipstack_close(kifp->fd);
		return ERROR;
	}
    /*TODO:MAC��ȡ */


    macadd[5] = ifp->ifindex & 0xff;
    if(pal_interface_set_lladdr(ifp, macadd, 6) != OK)
	{
		zlog_err(MODULE_PAL, "Unable to set L3 interface  mac %s(%s).",
				ifp->name, ipstack_strerror(ipstack_errno));
		ipstack_close(kifp->fd);
		return ERROR;
	}
	ipstack_set_nonblocking(kifp->fd);
	//ipkernel_register(kifp);

	return OK;
}

static int _linux_kernel_vlaneth_destroy (nsm_vlaneth_t *kifp)
{

	//struct ipstack_ifreq ifr;
	struct interface *ifp;
	if(!kifp)
		return -1;
	ifp = kifp->ifp;

	//TODO unregister this fd
	//ipkernel_unregister(kifp);
	pal_interface_down (ifp);
	ipstack_close (kifp->fd);
	//TODO to update if kernel index
	ifp->k_ifindex = 0;
	return OK;

}



static int ip_vlan_create(const char *name, vlan_t vid)
{
	int ret = -1;
	struct vlan_ioctl_args v_req;
	memset(&v_req, 0, sizeof(struct vlan_ioctl_args));
	v_req.cmd = ADD_VLAN_CMD;
	strcpy(v_req.device1,name);
	v_req.u.VID = vid;
	ret = _ipkernel_if_ioctl (IPSTACK_SIOCSIFVLAN, &v_req);
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
	ret = _ipkernel_if_ioctl (IPSTACK_SIOCSIFVLAN, &v_req);
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
	ret = _ipkernel_if_ioctl(SIOCSIFVLAN, &v_req);
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
	ret = _ipkernel_if_ioctl(SIOCSIFVLAN, &v_req);
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
	ret = _ipkernel_if_ioctl(SIOCSIFVLAN, &v_req);
	return ret;
}
#endif

static int _ipkernel_vlan_create (nsm_vlaneth_t *kifp)
{
	if(ip_vlan_create(kifp->root->k_name, kifp->vlanid) == 0)
	{
		return OK;
	}
	return ERROR;
}

static int _ipkernel_vlan_destroy (nsm_vlaneth_t *kifp)
{
	return ip_vlan_delete(kifp->root->k_name, kifp->vlanid);
}


int _ipkernel_vlaneth_create (nsm_vlaneth_t *kifp)
{
	if(if_is_serial(kifp->ifp))
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _linux_kernel_vlaneth_create(kifp);
		return OK;
	}
	if(if_is_ethernet(kifp->ifp)  && kifp->root &&
			IF_ID_GET(kifp->ifp->ifindex) &&
			IF_VLAN_GET(kifp->ifp->ifindex))
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _ipkernel_vlan_create(kifp);
		return OK;
	}
	return ERROR;
}

int _ipkernel_vlaneth_destroy (nsm_vlaneth_t *kifp)
{
	if(if_is_serial(kifp->ifp))
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _linux_kernel_vlaneth_destroy(kifp);
		return OK;
	}
	if(if_is_ethernet(kifp->ifp) && kifp->root && IF_ID_GET(kifp->ifp->ifindex) &&
			IF_VLAN_GET(kifp->ifp->ifindex) && kifp->active)
	{
		if(kifp->ifp->ll_type != IF_LLT_MODEM)
			return _ipkernel_vlan_destroy(kifp);
		return OK;
	}
	return ERROR;
}

int _ipkernel_vlaneth_change (nsm_vlaneth_t *kifp, vlan_t vlan)
{
	if(if_is_ethernet(kifp->ifp) && kifp->root && IF_ID_GET(kifp->ifp->ifindex))
	{
		if(kifp->ifp->ll_type == IF_LLT_MODEM)
			return OK;
		if(kifp->active)
			_ipkernel_vlan_destroy(kifp);
		kifp->vlanid = vlan;
		if(_ipkernel_vlan_create(kifp) == 0)
		{
			kifp->active = zpl_true;
			return OK;
		}
	}
	return ERROR;
}
#endif