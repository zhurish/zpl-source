/*
 * ipkernel_vlaneth.c
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "kernel_ioctl.h"
#define _LINUX_IP_H
#include <linux/if_tun.h>
#include "linux/if_vlan.h"
#include "linux/sockios.h"

#ifdef ZPL_NSM_VLANETH

#define IPKERNEL_TUN_NAME	"/dev/net/tun"
#define IPKERNEL_TAP_NAME	"/dev/tap"

static int _ipkernel_vlaneth_create (nsm_vlaneth_t *kifp)
{
	struct ifreq ifr;
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
		kifp->fd._fd = open(IPKERNEL_TUN_NAME, O_RDWR);
		if (kifp->fd._fd < 0)
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TUN_NAME, safe_strerror(errno));
			return ERROR;
		}
	}
	else
	{
		kifp->fd._fd = open(IPKERNEL_TAP_NAME, O_RDWR);
		if (kifp->fd._fd < 0)
		{
			zlog_err(MODULE_PAL, "Unable to open %s to create L3 interface(%s).",
					IPKERNEL_TUN_NAME, safe_strerror(errno));
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

	if (ioctl(kifp->fd._fd, TUNSETIFF, &ifr) < 0)
	{
		zlog_err(MODULE_PAL, "Unable to create corresponding L3 interface %s(%s).",
				ifp->name, safe_strerror(errno));
		close(kifp->fd._fd);
		return ERROR;
	}
    /*TODO:MAC��ȡ */


    macadd[5] = ifp->ifindex & 0xff;
    if(pal_interface_set_lladdr(ifp, macadd, 6) != OK)
	{
		zlog_err(MODULE_PAL, "Unable to set L3 interface  mac %s(%s).",
				ifp->name, safe_strerror(errno));
		close(kifp->fd._fd);
		return ERROR;
	}
	os_set_nonblocking(kifp->fd._fd);
	//ipkernel_register(kifp);

	return OK;
}

static int _ipkernel_vlaneth_destroy (nsm_vlaneth_t *kifp)
{

	//struct ifreq ifr;
	struct interface *ifp;
	if(!kifp)
		return -1;
	ifp = kifp->ifp;

	//TODO unregister this fd
	//ipkernel_unregister(kifp);
	pal_interface_down (ifp);
	close (kifp->fd._fd);
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
	ret = if_ioctl (SIOCSIFVLAN, &v_req);
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
	ret = if_ioctl (SIOCSIFVLAN, &v_req);
	//ret = ioctl(vlan_sock_fd, SIOCSIFVLAN, &v_req);
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
	ret = if_ioctl(SIOCSIFVLAN, &v_req);
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
	ret = if_ioctl(SIOCSIFVLAN, &v_req);
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
	ret = if_ioctl(SIOCSIFVLAN, &v_req);
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


int _ipkernel_linux_create (nsm_vlaneth_t *kifp)
{
	if(if_is_serial(kifp->ifp))
	{
		if(kifp->ifp->ll_type != ZEBRA_LLT_MODEM)
			return _ipkernel_vlaneth_create(kifp);
		return OK;
	}
	if(if_is_ethernet(kifp->ifp)  && kifp->root &&
			IF_ID_GET(kifp->ifp->ifindex) &&
			IF_VLAN_GET(kifp->ifp->ifindex))
	{
		if(kifp->ifp->ll_type != ZEBRA_LLT_MODEM)
			return _ipkernel_vlan_create(kifp);
		return OK;
	}
	return ERROR;
}

int _ipkernel_linux_destroy (nsm_vlaneth_t *kifp)
{
	if(if_is_serial(kifp->ifp))
	{
		if(kifp->ifp->ll_type != ZEBRA_LLT_MODEM)
			return _ipkernel_vlaneth_destroy(kifp);
		return OK;
	}
	if(if_is_ethernet(kifp->ifp) && kifp->root && IF_ID_GET(kifp->ifp->ifindex) &&
			IF_VLAN_GET(kifp->ifp->ifindex) && kifp->active)
	{
		if(kifp->ifp->ll_type != ZEBRA_LLT_MODEM)
			return _ipkernel_vlan_destroy(kifp);
		return OK;
	}
	return ERROR;
}

int _ipkernel_linux_change (nsm_vlaneth_t *kifp, vlan_t vlan)
{
	if(if_is_ethernet(kifp->ifp) && kifp->root && IF_ID_GET(kifp->ifp->ifindex))
	{
		if(kifp->ifp->ll_type == ZEBRA_LLT_MODEM)
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